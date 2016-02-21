package uk.co.wumpus.tinker.builder.arch;

@SuppressWarnings("serial")
public class ArchException extends Exception {
	
	public ArchException(final String msg) {
		super(msg);
	}
	
	public ArchException(final String msg, final Throwable cause) {
		super(msg, cause);
	}
}
