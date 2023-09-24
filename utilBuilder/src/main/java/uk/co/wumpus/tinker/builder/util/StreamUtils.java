package uk.co.wumpus.tinker.builder.util;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;

import javax.annotation.Nullable;

public final class StreamUtils {

	private StreamUtils() {
		/* hidden */
	}

	@Nullable
	public static String getStream(@Nullable final InputStream s) throws IOException {
		if (s == null) {
			return null;
		}
		byte[] stdIn = new byte[s.available()];
		s.read(stdIn);
		//TODO check output of read
		return new String(stdIn, Charset.defaultCharset());
	}
}
