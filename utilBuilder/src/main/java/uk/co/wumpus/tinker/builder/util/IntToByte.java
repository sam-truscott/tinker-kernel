package uk.co.wumpus.tinker.builder.util;

public final class IntToByte {

	private IntToByte() {
		/* hidden */
	}
	
	public static byte[] intToByte(final int i) {
		byte[] b = new byte[4];
		b[0] = (byte) ((i & 0xFF000000) >> 24);
		b[1] = (byte) ((i & 0xFF0000) >> 16);
		b[2] = (byte) ((i & 0xFF00) >> 8);
		b[3] = (byte) ((i & 0xFF));
		return b;
	}
}
