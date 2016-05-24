package uk.co.wumpus.tinker.builder.util;

import javax.annotation.Nonnull;

public final class IntToByte {

	private IntToByte() {
		/* hidden */
	}

	@Nonnull
	private static byte[] intToByteArrayBigEndian(int value) {
	    return new byte[] {
	            (byte)(value >>> 24),
	            (byte)(value >>> 16),
	            (byte)(value >>> 8),
	            (byte)value};
	}

	@Nonnull
	private static byte[] intToByteArrayLittleEndian(int value) {
	    return new byte[] {
	            (byte)(value),
	            (byte)(value >>> 8),
	            (byte)(value >>> 16),
	            (byte)(value >>> 24)};
	}
	
	@Nonnull
	public static byte[] intToByteArray(final int value, @Nonnull final Endian e) {
		switch (e) {
		case BIG:
			return intToByteArrayBigEndian(value);
		case SMALL:
			return intToByteArrayLittleEndian(value);
		default:
			throw new IllegalArgumentException("Unsupported endianness");
		}
	}
}
