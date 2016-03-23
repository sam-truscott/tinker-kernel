package uk.co.wumpus.tinker.builder.apps;

import java.io.File;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Elf extends Binary {

	private static final Logger LOG = LoggerFactory.getLogger(Elf.class);
	
	public Elf(File elfFile) throws ApplicationException {
		super(elfFile);
	}
	
	@Override
	public void copyTo(Payload payload) throws ApplicationException {
		try {
			payload.write(intToByteArrayLittleEndian(getData().length));
		} catch (Exception e) {
			throw new ApplicationException("Failed to write ELF length to payload", e);
		}
		super.copyTo(payload);
	}
	
	private static final byte[] intToByteArrayBigEndian(int value) {
	    return new byte[] {
	            (byte)(value >>> 24),
	            (byte)(value >>> 16),
	            (byte)(value >>> 8),
	            (byte)value};
	}
	    
    private static final byte[] intToByteArrayLittleEndian(int value) {
	    return new byte[] {
	            (byte)(value),
	            (byte)(value >>> 8),
	            (byte)(value >>> 16),
	            (byte)(value >>> 24)};
	}


	public void validate() throws ApplicationException {
		LOG.info("Validating ELF");
		// TODO - get/write ELF parser and validate
		LOG.info("Validation complete");
	}
}
