package uk.co.wumpus.tinker.builder.apps;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.arch.Arch;
import uk.co.wumpus.tinker.builder.arch.ArchException;
import uk.co.wumpus.tinker.builder.util.ByteToHex;

public class Payload {

	private static final Logger LOG = LoggerFactory.getLogger(Payload.class);
	private static final byte[] HEADER = new byte[256];
	private final RandomAccessFile fileWriter;
	private final Arch arch;
	private final Application kernel;
	private final List<Application> apps = new LinkedList<>();
	
	static {
		Arrays.fill(HEADER, (byte)0);
	}
	
	public Payload(
			final File payload,
			final Arch arch,
			final Application kernel) throws ApplicationException {
		LOG.info("Creating payload at {}", payload);
		this.arch = arch;
		this.kernel = kernel;
		try {
			this.fileWriter = new RandomAccessFile(payload, "rw");
		} catch (Exception e) {
			throw new ApplicationException("Failed to open the payload file for writing", e);
		}
	}
	
	public void addApplication(final Application app) {
		this.apps.add(app);
	}
	
	public void write(final byte[] b) throws IOException {
		LOG.info("Writing {} bytes to buffer", b.length);
		if (LOG.isTraceEnabled()) {
			LOG.trace("Writing {}", ByteToHex.bytesToHex(b));
		}
		this.fileWriter.write(b);
	}
	
	public void write(final byte[] b, final int off) throws IOException {
		LOG.info("Writing {} bytes to buffer at {}", b.length, off);
		if (LOG.isTraceEnabled()) {
			LOG.trace("Writing {}", ByteToHex.bytesToHex(b));
		}
		final long pos = this.fileWriter.getFilePointer();
		try {
			this.fileWriter.seek(off);
			this.fileWriter.write(b);
		} finally {
			this.fileWriter.seek(pos);
		}
	}
	
	public void writeToDisk() throws IOException, ApplicationException, ArchException {
		LOG.info("Writing buffer to disk");
		this.write(HEADER);
		int offset = HEADER.length;
		LOG.info("Offset is {}", offset);
		for (final Application app : this.apps) {
			offset += app.copyTo(this);
			LOG.info("Offset is {}", offset);
		}
		LOG.info("Final offset is {}", offset);
		this.kernel.copyTo(this);
		this.arch.writeBootstrap(this, offset);
	}
	
	public void close() {
		try {
			if (this.fileWriter != null) {
				this.fileWriter.close();
			}
		} catch (Exception e) {
			LOG.error("Failed to close the file writing buffer", e);
		}
	}
}
