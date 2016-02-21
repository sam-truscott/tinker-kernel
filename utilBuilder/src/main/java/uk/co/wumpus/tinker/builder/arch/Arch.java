package uk.co.wumpus.tinker.builder.arch;

import uk.co.wumpus.tinker.builder.apps.Payload;

public interface Arch {

	void writeBootstrap(final Payload payload, final int offset) throws ArchException;
	
	String toString();
}
