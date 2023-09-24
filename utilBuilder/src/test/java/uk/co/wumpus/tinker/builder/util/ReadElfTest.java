package uk.co.wumpus.tinker.builder.util;

import java.io.File;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Matchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.junit.Assert;

@RunWith(PowerMockRunner.class)
@PrepareForTest({ReadElf.class, ProcessBuilder.class})
@SuppressWarnings("static-method")
public class ReadElfTest {
	@Test
	public void test() throws Exception {		
		File in = File.createTempFile("in-test-", ".tmp");

		in.deleteOnExit();
		try {
			ProcessBuilder builder = PowerMockito.mock(ProcessBuilder.class);
			PowerMockito.whenNew(ProcessBuilder.class).withParameterTypes(String[].class).withArguments(Matchers.anyVararg()).thenReturn(builder);
			Process proc = Mockito.mock(Process.class);
			Mockito.when(builder.start()).thenReturn(proc);
			Mockito.when(proc.waitFor()).thenReturn(0);
			ReadElf copy = new ReadElf("test", in);
			Assert.assertTrue(copy.execute());
			
			Mockito.when(proc.waitFor()).thenReturn(1);
			Assert.assertFalse(copy.execute());
		} finally {
			in.delete();
		}
	}
}
