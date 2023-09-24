package uk.co.wumpus.tinker.builder.util;

import org.junit.Assert;
import org.junit.Test;

@SuppressWarnings("static-method")
public class EndianTest {

	@Test
	public void testConv() {
		Assert.assertEquals(Endian.SMALL, Endian.fromString("small"));
		Assert.assertEquals(Endian.BIG, Endian.fromString("big"));
	}
	
	@Test(expected=IllegalArgumentException.class)
	public void testConvNull() {
		Endian.fromString(null);
	}
	
	@Test(expected=IllegalArgumentException.class)
	public void testConvUnknown() {
		Endian.fromString("hello");
	}
}
