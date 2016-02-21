package uk.co.wumpus.tinker.builder.apps;

import java.io.File;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Binary implements Application {
	
	private static final Logger LOG = LoggerFactory.getLogger(Binary.class);
	private final File binary;
	
	public Binary(final File binaryFile) {
		LOG.info("Creating binary file for {}", binaryFile);
		this.binary = binaryFile;
	}

	public void copyTo(final Payload payload) throws ApplicationException {
		
	}
	
	public void validate() throws ApplicationException {
		
	}
}
