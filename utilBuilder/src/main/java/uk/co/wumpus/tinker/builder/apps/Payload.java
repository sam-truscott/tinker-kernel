package uk.co.wumpus.tinker.builder.apps;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.util.ByteToHex;

public class Payload {

	private static final Logger LOG = LoggerFactory.getLogger(Payload.class);
	private static final byte[] HEADER = new byte[128];
	private final RandomAccessFile fileWriter;
	private final Application kernel;
	private final List<Application> apps = new LinkedList<>();
	
	static {
		Arrays.fill(HEADER, (byte)0);
	}
	
	public Payload(
			final File payload,
			final Application kernel) throws ApplicationException {
		LOG.info("Creating payload at {}", payload);
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
			if (this.fileWriter != null) {
				this.fileWriter.close();
			}
		} catch (Exception e) {
			LOG.error("Failed to close the file writing buffer", e);
		}
	}
}
