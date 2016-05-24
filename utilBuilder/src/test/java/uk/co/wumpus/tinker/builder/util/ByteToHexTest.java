package uk.co.wumpus.tinker.builder.util;

import org.junit.Assert;
import org.junit.Test;

@SuppressWarnings("static-method")
public class ByteToHexTest {

	@Test
	public void testNull() {
		Assert.assertEquals("", ByteToHex.bytesToHex(null));
	}
	
	@Test
	public void testEmptyArray() {
		Assert.assertEquals("", ByteToHex.bytesToHex(new byte[]{}));
	}
	
	@Test
	public void testHexString() {
		Assert.assertEquals("00,01,02,03,04,05,06,07,08,09,0A,64,C8,", ByteToHex.bytesToHex(new byte[]{0,1,2,3,4,5,6,7,8,9,10,100,(byte)200}));
	}
}
