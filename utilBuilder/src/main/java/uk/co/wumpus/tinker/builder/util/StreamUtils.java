package uk.co.wumpus.tinker.builder.util;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;

public final class StreamUtils {

	private StreamUtils() {
		/* hidden */
	}

	public static String getStream(final InputStream s) throws IOException {
		byte[] stdIn = new byte[s.available()];
		s.read(stdIn);
		//TODO check output of read
		return new String(stdIn, Charset.defaultCharset());
	}
}
