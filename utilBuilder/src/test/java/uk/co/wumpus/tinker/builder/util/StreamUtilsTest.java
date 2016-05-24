package uk.co.wumpus.tinker.builder.util;

import java.io.IOException;

import org.junit.Assert;
import org.junit.Test;

@SuppressWarnings("static-method")
public class StreamUtilsTest {

	@Test
	public void testNull() throws IOException {
		Assert.assertNull(StreamUtils.getStream(null));
	}
}
