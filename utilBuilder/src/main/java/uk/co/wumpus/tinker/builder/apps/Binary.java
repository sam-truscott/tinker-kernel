package uk.co.wumpus.tinker.builder.apps;

import java.io.File;
import java.io.FileInputStream;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Binary implements Application {
	
	private static final Logger LOG = LoggerFactory.getLogger(Binary.class);
	private final File binary;
	
	public Binary(final File binaryFile) {
		LOG.info("Creating binary file for {}", binaryFile);
		this.binary = binaryFile;
	}

	@Override
	public String toString() {
		return this.binary.toString();
	}

	public int copyTo(final Payload payload) throws ApplicationException {
		LOG.info("Copying binary {} to payload", this.binary);
		FileInputStream fis = null;
		int len = 0;
		try {
			fis = new FileInputStream(this.binary);
			final byte[] data = new byte[fis.available()];
			len = fis.read(data);
			LOG.info("Read {} bytes from {}", len, this.binary);
			if (-1 == len) {
				LOG.error("Failed to read data from binary");
			} else {
				payload.write(data);
			}
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
		return len;
	}
	
	public void validate() throws ApplicationException {
		
	}
}
