package uk.co.wumpus.tinker.builder.apps;

import java.io.File;
import java.util.Arrays;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Matchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.junit.Assert;

import uk.co.wumpus.tinker.builder.util.ReadElf;

@RunWith(PowerMockRunner.class)
@PrepareForTest({ReadElf.class, ProcessBuilder.class})
@PowerMockIgnore({
		"java.lang.*",
		"org.objenesis.*"})
public class ElfTest {

	private Process proc;
	
	@Before
	public void setup() throws Exception {
		ProcessBuilder builder = PowerMockito.mock(ProcessBuilder.class);
		PowerMockito.whenNew(ProcessBuilder.class).withParameterTypes(String[].class).withArguments(Matchers.anyVararg()).thenReturn(builder);
		proc = Mockito.mock(Process.class);
		Mockito.when(builder.start()).thenReturn(proc);
		Mockito.when(proc.waitFor()).thenReturn(0);
	}
	
	@SuppressWarnings({ "null", "static-method" })
	@Test
	public void test() throws Exception {
		final File f = File.createTempFile("tmp-file-", ".tmp");
		f.deleteOnExit();
		try {
			Elf b = new Elf(f, "myarch");
			Assert.assertTrue(Arrays.equals(new byte[]{}, b.getData()));
			Assert.assertEquals(f, b.getFile());
			b.validate();
			Payload p = Mockito.mock(Payload.class);
			b.copyTo(p);
			Mockito.verify(p, Mockito.times(1)).write(Matchers.eq(new byte[]{}));
		} finally {
			f.delete();
		}
	}
	
	@SuppressWarnings({ })
	@Test(expected=ApplicationException.class)
	public void testValidationFailure() throws Exception {
		final File f = File.createTempFile("tmp-file-", ".tmp");
		f.deleteOnExit();
		try {
			Elf b = new Elf(f, "myarch");
			Mockito.when(proc.waitFor()).thenReturn(1);
			b.validate();
		} finally {
			f.delete();
		}
	}
}
