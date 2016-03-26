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
import uk.co.wumpus.tinker.builder.util.Objcopy;

public class TinkerBuilder {

	private static final Logger LOG = LoggerFactory.getLogger(TinkerBuilder.class);
	
	public static void main(final String... args) throws Exception {
		LOG.info("TinkerBuilder");
		if (args.length < 4) {
			LOG.error("Need to specify output, arch (e.g. arm-eabi-), endianness (big or small) and kernel image as a minimum");
			return;
		}

		Endian endianness = Endian.fromString(args[2]);
		final File kernelElf = new File(args[3]);
		final File kernelBinary = File.createTempFile("tinker_kernel-", ".img");
		kernelBinary.deleteOnExit();
		LOG.info("Using kernel file {} to generate binary {}", kernelElf, kernelBinary);
		final String archPrefix = args[1];
		final Objcopy converter = new Objcopy(archPrefix, kernelElf, kernelBinary);
		if (!converter.execute()) {
			LOG.error("Failed to convert the kernel from ELF to a binary, aborting.");
			return;
		}
		
		final File outputFile = new File(args[0]);
		if (outputFile.exists() && !outputFile.delete()) {
			LOG.error("Failed to delete the existing package image, aborting.");
			return;
		}
		final Payload payload = new Payload(outputFile, new Binary(kernelBinary));
		final List<String> argList = new ArrayList<>(Arrays.asList(args));
		argList.remove(0);
		argList.remove(0);
		argList.remove(0);
		argList.remove(0);
		for (final String elf : argList) {
			payload.addApplication(new Elf(new File(elf), endianness, archPrefix));
		}
		try {
			payload.writeToDisk();
		} finally {
			payload.close();
		}
	}
}
