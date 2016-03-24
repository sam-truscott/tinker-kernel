package uk.co.wumpus.tinker.builder;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.apps.Binary;
import uk.co.wumpus.tinker.builder.apps.Elf;
import uk.co.wumpus.tinker.builder.apps.Payload;
import uk.co.wumpus.tinker.builder.util.Endian;

public class TinkerBuilder {

	private static final Logger LOG = LoggerFactory.getLogger(TinkerBuilder.class);
	
	public static void main(final String... args) throws Exception {
		LOG.info("TinkerBuilder");
		if (args.length < 3) {
			LOG.error("Need to specify output, endianness (big or small) and kernel image as a minimum");
			return;
		}
		final Binary kernel = new Binary(new File(args[1]));
		LOG.info("Using kernel file {}", kernel);
		final Payload payload = new Payload(new File(args[0]), kernel);
		Endian endianness = Endian.fromString(args[2]);
		final List<String> argList = new ArrayList<>(Arrays.asList(args));
		argList.remove(0);
		argList.remove(0);
		argList.remove(0);
		for (final String elf : argList) {
			payload.addApplication(new Elf(new File(elf), endianness));
		}
		try {
			payload.writeToDisk();
		} finally {
			payload.close();
		}
	}
}
