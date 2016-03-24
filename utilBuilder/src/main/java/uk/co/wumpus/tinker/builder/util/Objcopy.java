package uk.co.wumpus.tinker.builder.util;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Objcopy {

	private static final Logger LOG = LoggerFactory.getLogger(Objcopy.class);
	private final String commandLine;
	private final File inputFile;
	private final File outputFile;
	
	public Objcopy(
			final String archPrefix,
			final File in,
			final File out) {
		this.commandLine = archPrefix + "-objcopy";
		this.inputFile = in;
		this.outputFile = out;
	}
	
	public boolean execute() throws IOException, InterruptedException {
		LOG.info("Starting process {}", this.commandLine);
		Process proc = new ProcessBuilder(
				this.commandLine,
				"-O",
				"binary",
				this.inputFile.getAbsoluteFile().toString(),
				this.outputFile.getAbsoluteFile().toString())
				.start();
		LOG.info("Started {}", this.commandLine);
		final int returnCode = proc.waitFor();
		LOG.info("{} finished with return code {}", this.commandLine, returnCode);
		LOG.info("Output [{}]", getStream(proc.getInputStream()));
		LOG.info("Error [{}]", getStream(proc.getErrorStream()));
		return returnCode == 0;
	}

	private static String getStream(final InputStream s) throws IOException {
		byte[] stdIn = new byte[s.available()];
		s.read(stdIn);
		return new String(stdIn);
	}
}
