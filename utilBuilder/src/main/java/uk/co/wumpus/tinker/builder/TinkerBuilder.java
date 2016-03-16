package uk.co.wumpus.tinker.builder;

import java.io.File;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.apps.Binary;
import uk.co.wumpus.tinker.builder.apps.Payload;

public class TinkerBuilder {

	private static final Logger LOG = LoggerFactory.getLogger(TinkerBuilder.class);
	
	public static void main(final String... args) throws Exception {
		LOG.info("TinkerBuilder");
		if (args.length < 2) {
			LOG.error("Need to specify output and kernel image as a minimum");
			return;
		}
		final Binary kernel = new Binary(new File(args[1]));
		LOG.info("Using kernel file {}", kernel);
		final Payload payload = new Payload(new File(args[0]), kernel);		
		try {
			payload.writeToDisk();
		} finally {
			payload.close();
		}
	}
}
