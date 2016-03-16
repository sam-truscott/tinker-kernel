package uk.co.wumpus.tinker.builder.apps;

import java.io.File;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Elf extends Binary {

	private static final Logger LOG = LoggerFactory.getLogger(Elf.class);
	
	public Elf(File elfFile) {
		super(elfFile);
	}

	public void validate() throws ApplicationException {
		LOG.info("Validating ELF");
		// TODO - get/write ELF parser and validate
		LOG.info("Validation complete");
	}
}
