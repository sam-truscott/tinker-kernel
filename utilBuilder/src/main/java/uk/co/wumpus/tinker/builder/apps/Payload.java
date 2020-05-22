package uk.co.wumpus.tinker.builder.apps;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.util.ByteToHex;
import uk.co.wumpus.tinker.builder.util.Endian;
import uk.co.wumpus.tinker.builder.util.IntToByte;

public class Payload implements AutoCloseable {

	private static final Logger LOG = LoggerFactory.getLogger(Payload.class);
	@Nonnull private static final byte[] HEADER = new byte[128];
	@Nonnull private final RandomAccessFile fileWriter;
	@Nonnull private final Application kernel;
	@Nonnull private final Endian endian;
	@Nonnull private final List<Application> apps = new LinkedList<>();
	
	static {
		Arrays.fill(HEADER, (byte)0);
	}
	
	public Payload(
			@Nonnull final Endian endian,
			@Nonnull final File payload,
			@Nonnull final Application kernel) throws ApplicationException {
		LOG.debug("Creating payload at {} with endianness {}", payload, endian);
		this.endian = endian;
		this.kernel = kernel;
		try {
			this.fileWriter = new RandomAccessFile(payload, "rw");
		} catch (Exception e) {
			throw new ApplicationException("Failed to open the payload file for writing", e);
		}
	}
	
	public void addApplication(@Nonnull final Application app) {
		this.apps.add(app);
	}
	
	void write(@Nonnull final byte[] b) throws IOException {
		LOG.debug("Writing {} bytes to buffer at {}", b.length, this.length());
		if (LOG.isTraceEnabled()) {
			LOG.trace("Writing {}", ByteToHex.bytesToHex(b));
		}
		this.fileWriter.write(b);
	}
	
	public void writeToDisk() throws IOException, ApplicationException {
		LOG.debug("Writing buffer to disk");
		this.kernel.validate();
		this.kernel.copyTo(this);
		final int kernelLength = this.length();
		final int afterLengths = kernelLength + (4 * (this.apps.size() + 1));
		int aligned = align(afterLengths);
		LOG.info("Kernel finishes at 0x{}, lengths to 0x{}, aligning to 0x{}", 
				Integer.toString(kernelLength, 16), 
				Integer.toString(afterLengths, 16), 
				Integer.toString(aligned, 16)); 
		final Map<Application, Integer> offsets = new HashMap<>();
		final List<Integer> alignedOffsets = new ArrayList<>(this.apps.size());
		// calculate the aligned offsets of all the apps
		for (final Application app : this.apps) {
			app.validate();
			final int alignedOffset = aligned - kernelLength;
			LOG.info("Process {} starts at 0x{} ({}/0x{})", app.toString(), 
					Integer.toString(aligned, 16), 
					alignedOffset,
					Integer.toString(alignedOffset, 16));
			alignedOffsets.add(alignedOffset);
			offsets.put(app, aligned);
			aligned += align(aligned + app.length());
		}
		// write the length of the whole package
		final int endOfApps = aligned - kernelLength;
		LOG.info("Applications end at offset 0x{}", Integer.toString(endOfApps, 16));
		write(IntToByte.intToByteArray(endOfApps, this.endian));
		// write the lengths of all the apps
		for (final int offset : alignedOffsets) {
			write(IntToByte.intToByteArray(offset, this.endian));
		}
		// end of apps
		write(new byte[]{0,0,0,0});
		
		LOG.debug("Writing apps to payload");
		// write each app to the package
		for (final Application app : this.apps) {
			final int appAligned = offsets.get(app);
			byte[] fill = new byte[appAligned - length()];
			Arrays.fill(fill, (byte)0);
			write(fill);
			app.copyTo(this);
		}
		this.write(HEADER);
	}
	
	private static int align(final int value) {
		final int whole = value / 4096;
		final int up = whole * 4096;
		return (up >= value) ? up : up + 4096;
	}
	
	@Override
	public void close() {
		try {
			this.fileWriter.close();
		} catch (Exception e) {
			LOG.error("Failed to close the file writing buffer", e);
		}
	}
	
	public int length() throws IOException {
		return (int)this.fileWriter.length();
	}
}
