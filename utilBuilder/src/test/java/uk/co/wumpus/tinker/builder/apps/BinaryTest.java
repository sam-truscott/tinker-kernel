package uk.co.wumpus.tinker.builder.apps;

import java.io.File;

import org.junit.Test;
import org.mockito.Matchers;
import org.mockito.Mockito;
import org.testng.Assert;

public class BinaryTest {

	@SuppressWarnings({ "null", "static-method" })
	@Test
	public void test() throws Exception {
		final File f = File.createTempFile("tmp-file-", ".tmp");
		f.deleteOnExit();
		try {
			Binary b = new Binary(f);
			Assert.assertEquals(new byte[]{}, b.getData());
			Assert.assertEquals(f, b.getFile());
			b.validate();
			Payload p = Mockito.mock(Payload.class);
			b.copyTo(p);
			Mockito.verify(p, Mockito.times(1)).write(Matchers.eq(new byte[]{}));
		} finally {
			f.delete();
		}
	}
}
