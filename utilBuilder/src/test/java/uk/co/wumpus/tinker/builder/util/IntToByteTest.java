package uk.co.wumpus.tinker.builder.util;

import java.util.Arrays;

import org.junit.Assert;
import org.junit.Test;

@SuppressWarnings("static-method")
public class IntToByteTest {
	
	@Test
	public void testConvertZero() {
		Assert.assertTrue(Arrays.equals(new byte[]{0,0,0,0}, IntToByte.intToByteArray(0, Endian.SMALL)));
	}
	
	@Test
	public void testConvertOneSmall() {
		Assert.assertTrue(Arrays.equals(new byte[]{1,0,0,0}, IntToByte.intToByteArray(1, Endian.SMALL)));
	}
	
	@Test
	public void testConvertOneBig() {
		Assert.assertTrue(Arrays.equals(new byte[]{0,0,0,1}, IntToByte.intToByteArray(1, Endian.BIG)));
	}
	
	@Test
	public void testConvert256Small() {
		Assert.assertTrue(Arrays.equals(new byte[]{0,1,0,0}, IntToByte.intToByteArray(256, Endian.SMALL)));
	}
	
	@Test
	public void testConvert256Big() {
		Assert.assertTrue(Arrays.equals(new byte[]{0,0,1,0}, IntToByte.intToByteArray(256, Endian.BIG)));
	}
	
	@Test
	public void testConvert1S16Small() {
		Assert.assertTrue(Arrays.equals(new byte[]{0,0,1,0}, IntToByte.intToByteArray(1<<16, Endian.SMALL)));
	}
	
	@Test
	public void testConvert1S16Big() {
		Assert.assertTrue(Arrays.equals(new byte[]{0,1,0,0}, IntToByte.intToByteArray(1<<16, Endian.BIG)));
	}
	
	@Test
	public void testConvert1S24Small() {
		Assert.assertTrue(Arrays.equals(new byte[]{0,0,0,1}, IntToByte.intToByteArray(1<<24, Endian.SMALL)));
	}
	
	@Test
	public void testConvert1S24Big() {
		Assert.assertTrue(Arrays.equals(new byte[]{1,0,0,0}, IntToByte.intToByteArray(1<<24, Endian.BIG)));
	}
}
