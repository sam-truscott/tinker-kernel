package uk.co.wumpus.tinker.builder.apps;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Binary implements Application {
	
	private static final Logger LOG = LoggerFactory.getLogger(Binary.class);
	private final File binary;
	private final byte[] data;
	
	public Binary(final File binaryFile) throws ApplicationException {
		LOG.info("Creating binary file for {}", binaryFile);
		this.binary = binaryFile;
		this.data = read(this.binary);
	}

	@Override
	public String toString() {
		return this.binary.toString();
	}
	
	protected byte[] getData() {
		return this.data;
	}
	
	private static byte[] read(final File binary) throws ApplicationException {
		FileInputStream fis = null;
		try {
			fis = new FileInputStream(binary);
			final byte[] data = new byte[fis.available()];
			final int len = fis.read(data);
			LOG.info("Read {} bytes from {}", len, binary);
			if (-1 == len) {
				LOG.error("Failed to read data from binary");
			}
			return data;
		} catch (Exception e) {
			throw new ApplicationException("Failed to copy the binary into the payload", e);
		} finally {
			if (fis != null) {
				try {
					fis.close();
				} catch (Exception e) {
					LOG.error("Failed to close binary file", e);
				}
			}
		}
	}

	public void copyTo(final Payload payload) throws ApplicationException {
		LOG.info("Copying binary {} to payload", this.binary);
		try {
			payload.write(this.data);
		} catch (Exception e) {
			throw new ApplicationException("Failed to write bytes to payload", e);
		}
	}
	
	public void validate() throws ApplicationException {
		
	}
}
