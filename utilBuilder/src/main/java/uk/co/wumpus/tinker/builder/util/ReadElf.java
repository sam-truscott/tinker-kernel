package uk.co.wumpus.tinker.builder.util;

import java.io.File;
import java.io.IOException;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ReadElf {

	private static final Logger LOG = LoggerFactory.getLogger(ReadElf.class);
	private final String commandLine;
	private final File inputFile;
	
	public ReadElf(
			@Nonnull final String archPrefix,
			@Nonnull final File in) {
		this.commandLine = archPrefix + "-readelf";
		this.inputFile = in;
	}
	
	public boolean execute() throws IOException, InterruptedException {
		LOG.info("Starting process {}", this.commandLine);
		Process proc = new ProcessBuilder(
				this.commandLine,
				"-h",
				this.inputFile.getAbsoluteFile().toString())
				.start();
		LOG.info("Started {}", this.commandLine);
		final int returnCode = proc.waitFor();
		LOG.info("{} finished with return code {}", this.commandLine, returnCode);
		LOG.info("Output [{}]", StreamUtils.getStream(proc.getInputStream()));
		LOG.info("Error [{}]", StreamUtils.getStream(proc.getErrorStream()));
		return returnCode == 0;
	}
}
