package uk.co.wumpus.tinker.builder.arch;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import uk.co.wumpus.tinker.builder.apps.Payload;
import uk.co.wumpus.tinker.builder.util.ByteToHex;
import uk.co.wumpus.tinker.builder.util.IntToByte;

public class Arm implements Arch {

	private static final Logger LOG = LoggerFactory.getLogger(Arm.class);
	
	// e1a0000f        mov     r0, pc
	private static final byte[] GET_PC = new byte[] {(byte) 0xE1, (byte) 0xA0, 00, 0x0F};
	// e59f1004        ldr     r1, [pc, #4]    ; 10 <.text+0x10>
	private static final byte[] LOAD_OFFSET = new byte[] {(byte) 0xE5, (byte) 0x9F, 0x10, 0x04};
	// e0801001        add     r1, r0, r1
	private static final byte[] ADD_OFFSET_TO_PC = new byte[] {(byte) 0xE0, (byte) 0x80, 0x10, 0x01};
	// e1a0f001        mov     pc, r1
	private static final byte[] MOVE_OFFSET_TO_PC = new byte[] {(byte) 0xE1, (byte) 0xA0, (byte) 0xF0, 0x01};
	// offset
	
	public void writeBootstrap(final Payload payload, final int offset) throws ArchException {
		LOG.info("Writing bootstrap to payload");
		try {
			if (LOG.isDebugEnabled()) {
				LOG.debug(ByteToHex.bytesToHex(GET_PC));
				LOG.debug(ByteToHex.bytesToHex(LOAD_OFFSET));
				LOG.debug(ByteToHex.bytesToHex(ADD_OFFSET_TO_PC));
				LOG.debug(ByteToHex.bytesToHex(MOVE_OFFSET_TO_PC));
				LOG.debug(ByteToHex.bytesToHex(IntToByte.intToByte(offset)));
			}
			payload.write(GET_PC, 0);
			payload.write(LOAD_OFFSET, 4);
			payload.write(ADD_OFFSET_TO_PC, 8);
			payload.write(MOVE_OFFSET_TO_PC, 12);
			payload.write(IntToByte.intToByte(offset), 16);
		} catch (Exception e) {
			throw new ArchException("Failed to write bootstrap to payload", e);
		}
	}
	
	@Override
	public String toString() {
		return "arm";
	}
}
