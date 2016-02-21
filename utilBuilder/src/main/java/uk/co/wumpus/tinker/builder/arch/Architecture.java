package uk.co.wumpus.tinker.builder.arch;

import uk.co.wumpus.tinker.builder.apps.Payload;

public interface Architecture {

	void writeBootstrap(final Payload payload);
}
