package uk.co.wumpus.tinker.builder.apps;

import javax.annotation.Nonnull;

public interface Application {

	void copyTo(@Nonnull final Payload payload) throws ApplicationException;
	
	void validate() throws ApplicationException;
	
	int length();
}
