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
		LOG.debug("TinkerBuilder");
		if (args.length < 4) {
			LOG.error("Need to specify output, arch (e.g. arm-eabi), endianness (big or small) and kernel image as a minimum");
			return;
		}
		final String archPrefix = args[1];
		if (archPrefix == null || archPrefix.isEmpty()) {
			LOG.error("Architecture is empty or missing");
			return;
		}

		final Endian endianness = Endian.fromString(args[2]);
		final File kernelElf = new File(args[3]);
		if (!kernelElf.canRead()) {
			LOG.error("Cannot read {}", args[3]);
			return;
		}
		final File kernelBinary = File.createTempFile("tinker_kernel-", ".img");
		kernelBinary.deleteOnExit();
		LOG.debug("Using kernel file {} to generate binary {}", kernelElf, kernelBinary);
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
		try (final Payload payload = new Payload(endianness, outputFile, new Binary(kernelBinary))) {
			final List<String> argList = new ArrayList<>(Arrays.asList(args));
			argList.remove(0);
			argList.remove(0);
			argList.remove(0);
			argList.remove(0);
			for (final String elf : argList) {
				payload.addApplication(new Elf(new File(elf), archPrefix));
			}
			LOG.debug("Writing payload to disk");
			payload.writeToDisk();
		}
		LOG.debug("Written");
	}
}
