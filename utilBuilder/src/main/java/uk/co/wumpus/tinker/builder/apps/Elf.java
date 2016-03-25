package uk.co.wumpus.tinker.builder.apps;

import java.io.File;
import java.io.IOException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.util.Endian;
import uk.co.wumpus.tinker.builder.util.IntToByte;
import uk.co.wumpus.tinker.builder.util.ReadElf;

public class Elf extends Binary {

	private static final Logger LOG = LoggerFactory.getLogger(Elf.class);
	private final Endian endianness;
	private final String archPrefix;
	
	public Elf(
			final File elfFile, 
			final Endian endianness,
			final String archPrefix) throws ApplicationException {
		super(elfFile);
		this.endianness = endianness;
		this.archPrefix = archPrefix;
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
			payload.write(IntToByte.intToByteArray(getData().length, endianness));
		} catch (Exception e) {
			throw new ApplicationException("Failed to write ELF length to payload", e);
		}
		super.copyTo(payload);
	}

	public void validate() throws ApplicationException {
		LOG.info("Validating ELF");
		ReadElf e = new ReadElf(this.archPrefix, super.getFile());
		try {
			if (!e.execute()) {
				throw new ApplicationException("This doesn't seem like a valid ELF file");
			}
		} catch (InterruptedException | IOException e2) {
			throw new ApplicationException("Validation task threw an exception", e2);
		}
		LOG.info("Validation complete");
	}
}
