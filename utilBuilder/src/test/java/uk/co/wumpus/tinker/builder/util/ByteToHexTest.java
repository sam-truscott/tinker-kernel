package uk.co.wumpus.tinker.builder.util;

import org.junit.Assert;
import org.junit.Test;

public class ByteToHexTest {

	@Test
	public void testNull() {
		Assert.assertEquals("", ByteToHex.bytesToHex(null));
	}
	
	@Test
	public void testEmptyArray() {
		Assert.assertEquals("", ByteToHex.bytesToHex(new byte[]{}));
	}
}
