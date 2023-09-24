package uk.co.wumpus.tinker.builder.util;

import java.util.HashMap;
import java.util.Map;

import javax.annotation.Nonnull;

public enum Endian {
	BIG("big"),
	SMALL("small");
	
	private static Map<String, Endian> MAP;
	private final String name;
	
	Endian(final String e) {
		this.name = e;
		add(e, this);
	}

	private static void add(final String e, final Endian end) {
		if (MAP == null) {
			MAP = new HashMap<>(2);
		}
		MAP.put(e, end);
	}

	@Nonnull
	public static Endian fromString(final String e) {
		final Endian endianness = MAP.get(e);
		if (endianness == null) {
			throw new IllegalArgumentException("Unsupported");
		}
		return endianness;
	}
	
	@Override
	public String toString() {
		return this.name;
	}
}
