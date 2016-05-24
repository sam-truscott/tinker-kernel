package uk.co.wumpus.tinker.builder.apps;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.util.ByteToHex;

public class Payload {

	private static final Logger LOG = LoggerFactory.getLogger(Payload.class);
	@Nonnull private static final byte[] HEADER = new byte[128];
	@Nonnull private final RandomAccessFile fileWriter;
	@Nonnull private final Application kernel;
	@Nonnull private final List<Application> apps = new LinkedList<>();
	
	static {
		Arrays.fill(HEADER, (byte)0);
	}
	
	public Payload(
			@Nonnull final File payload,
			@Nonnull final Application kernel) throws ApplicationException {
		LOG.info("Creating payload at {}", payload);
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
		LOG.info("Writing {} bytes to buffer", b.length);
		if (LOG.isTraceEnabled()) {
			LOG.trace("Writing {}", ByteToHex.bytesToHex(b));
		}
		this.fileWriter.write(b);
	}
	
	public void writeToDisk() throws IOException, ApplicationException {
		LOG.info("Writing buffer to disk");
		this.kernel.validate();
		this.kernel.copyTo(this);
		for (final Application app : this.apps) {
			app.validate();
			app.copyTo(this);
		}
		this.write(HEADER);
	}
	
	public void close() {
		try {
			this.fileWriter.close();
		} catch (Exception e) {
			LOG.error("Failed to close the file writing buffer", e);
		}
	}
	
	public long length() throws IOException {
		return this.fileWriter.length();
	}
}
