package uk.co.wumpus.tinker.builder.apps;

public interface Application {

	void copyTo(final Payload payload) throws ApplicationException;
	
	void validate() throws ApplicationException;
}
