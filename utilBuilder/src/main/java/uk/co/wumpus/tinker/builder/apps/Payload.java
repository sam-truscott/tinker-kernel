package uk.co.wumpus.tinker.builder.apps;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.util.ByteToHex;

public class Payload {

	private static final Logger LOG = LoggerFactory.getLogger(Payload.class);
	private final ByteArrayOutputStream writer;
	private final OutputStream fileWriter;
	
	public Payload(final File payload) throws ApplicationException {
		LOG.info("Creating payload at {}", payload);
		try {
			this.writer = new ByteArrayOutputStream();
			this.fileWriter = new FileOutputStream(payload);
		} catch (Exception e) {
			throw new ApplicationException("Failed to open the payload file for writing", e);
		}
	}
	
	public void write(final byte[] b) throws IOException {
		LOG.info("Writing {} bytes to buffer", b.length);
		if (LOG.isDebugEnabled()) {
			LOG.debug("Writing {}", ByteToHex.bytesToHex(b));
		}
		this.writer.write(b);
	}
	
	public void write(final byte[] b, final int off, final int len) throws IOException {
		LOG.info("Writing {} bytes to buffer at {} with length {}", b.length, off, len);
		if (LOG.isDebugEnabled()) {
			LOG.debug("Writing {}", ByteToHex.bytesToHex(b));
		}
		this.writer.write(b, off, len);
	}
	
	public void writeToDisk() throws IOException {
		LOG.info("Writing buffer to disk");
		this.writer.writeTo(this.fileWriter);
	}
}
