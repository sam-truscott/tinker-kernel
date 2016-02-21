package uk.co.wumpus.tinker.builder.apps;

@SuppressWarnings("serial")
public class ApplicationException extends Exception {

	public ApplicationException(final String msg) {
		super(msg);
	}
	
	public ApplicationException(final String msg, final Throwable cause) {
		super(msg, cause);
	}
}
