package uk.co.wumpus.tinker.builder.apps;

import java.io.File;
import java.io.IOException;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.util.ReadElf;

public class Elf extends Binary {

	private static final Logger LOG = LoggerFactory.getLogger(Elf.class);
	@Nonnull private final String archPrefix;
	
	public Elf(
			@Nonnull final File elfFile, 
			@Nonnull final String archPrefix) throws ApplicationException {
		super(elfFile);
		this.archPrefix = archPrefix;
	}

	@Override
	public void validate() throws ApplicationException {
		LOG.debug("Validating ELF");
		ReadElf e = new ReadElf(this.archPrefix, super.getFile());
		try {
			if (!e.execute()) {
				throw new ApplicationException("This doesn't seem like a valid ELF file");
			}
		} catch (InterruptedException | IOException e2) {
			throw new ApplicationException("Validation task threw an exception", e2);
		}
		LOG.debug("Validation complete");
	}
}
