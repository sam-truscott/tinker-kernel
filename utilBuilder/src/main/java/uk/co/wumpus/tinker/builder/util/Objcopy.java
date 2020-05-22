package uk.co.wumpus.tinker.builder.util;

import java.io.File;
import java.io.IOException;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Objcopy {

	private static final Logger LOG = LoggerFactory.getLogger(Objcopy.class);
	private final String commandLine;
	private final File inputFile;
	private final File outputFile;
	
	public Objcopy(
			@Nonnull final String archPrefix,
			@Nonnull final File in,
			@Nonnull final File out) {
		this.commandLine = archPrefix + "-objcopy";
		this.inputFile = in;
		this.outputFile = out;
	}
	
	public boolean execute() throws IOException, InterruptedException {
		LOG.debug("Starting process {}", this.commandLine);
		Process proc = new ProcessBuilder(
				this.commandLine,
				"-O",
				"binary",
				this.inputFile.getAbsoluteFile().toString(),
				this.outputFile.getAbsoluteFile().toString())
				.start();
		LOG.debug("Started {}", this.commandLine);
		final int returnCode = proc.waitFor();
		LOG.debug("{} finished with return code {}", this.commandLine, returnCode);
		LOG.debug("Output [{}]", StreamUtils.getStream(proc.getInputStream()));
		LOG.debug("Error [{}]", StreamUtils.getStream(proc.getErrorStream()));
		return returnCode == 0;
	}
}
