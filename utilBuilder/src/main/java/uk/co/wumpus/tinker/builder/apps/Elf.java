package uk.co.wumpus.tinker.builder.apps;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Elf implements Application {
	
	private static final Logger LOG = LoggerFactory.getLogger(Elf.class);
	
	public int copyTo(final Payload payload) throws ApplicationException {
		return 0;
	}
	
	public void validate() throws ApplicationException {
		
	}
}
