package uk.co.wumpus.tinker.builder.util;

public final class ByteToHex {
	
	private static final char[] HEX_ARRAY = "0123456789ABCDEF".toCharArray();
	
	private ByteToHex() {
		/* hidden */
	}
	
	public static String bytesToHex(byte[] bytes) {
	    final StringBuilder builder = new StringBuilder();
	    for (int j = 0; j < bytes.length; j++) {
	        int v = bytes[j] & 0xFF;
	        builder.append(HEX_ARRAY[v >>> 4]);
	        builder.append(HEX_ARRAY[v & 0x0F]);
	        builder.append(',');
	        if (j != 0 && (j % 16) == 0) {
	        	builder.append('\n');
	        }
	    }
	    return builder.toString();
	}
}