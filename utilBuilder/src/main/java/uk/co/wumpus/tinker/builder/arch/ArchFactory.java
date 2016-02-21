package uk.co.wumpus.tinker.builder.arch;

public final class ArchFactory {

	private ArchFactory() {
		/* ignore */
	}
	
	public static Arch getArchitecture(final String arch) throws ArchException {
		switch (arch) {
			case "arm":
				return new Arm();
			default:
				throw new ArchException(arch + " is an unsupported architecture");
		}
	}
}
