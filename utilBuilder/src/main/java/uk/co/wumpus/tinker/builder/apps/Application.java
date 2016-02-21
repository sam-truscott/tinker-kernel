package uk.co.wumpus.tinker.builder.apps;

public interface Application {

	int copyTo(final Payload payload) throws ApplicationException;
	
	void validate() throws ApplicationException;
}
