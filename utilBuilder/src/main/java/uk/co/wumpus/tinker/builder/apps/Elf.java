package uk.co.wumpus.tinker.builder.apps;

import java.io.File;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.util.Endian;

public class Elf extends Binary {

	private static final Logger LOG = LoggerFactory.getLogger(Elf.class);
	private final Endian endianness;
	
	public Elf(final File elfFile, final Endian endianness) throws ApplicationException {
		super(elfFile);
		this.endianness = endianness;
	}
	
	@Override
	public void copyTo(Payload payload) throws ApplicationException {
		try {
			// align to a word boundary
			LOG.info("Current length is {}", payload.length());
			while ((payload.length() % 4) != 0)
			{
				LOG.info("Writing an alignment byte");
				payload.write(new byte[] {0});
			}
			payload.write(intToByteArray(getData().length, endianness));
		} catch (Exception e) {
			throw new ApplicationException("Failed to write ELF length to payload", e);
		}
		super.copyTo(payload);
	}
	
	private static final byte[] intToByteArray(final int value, final Endian e) {
		switch (e) {
		case BIG:
			return intToByteArrayBigEndian(value);
		case SMALL:
			return intToByteArrayLittleEndian(value);
		default:
			throw new IllegalArgumentException("Unsupported endianness");
		}
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
