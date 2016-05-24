package uk.co.wumpus.tinker.builder.apps;

import java.io.File;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Matchers;
import org.mockito.Mockito;

public class PayloadTest {

	@SuppressWarnings({ "static-method", "null" })
	@Test
	public void testPayload() throws Exception {
		final Application kernel = Mockito.mock(Application.class);
		final File image = File.createTempFile("temp-image-", ".tmp");
		image.deleteOnExit();
		try {
			Payload pl = new Payload(image, kernel);
			final Application app1 = Mockito.mock(Application.class);
			final Application app2 = Mockito.mock(Application.class);
			pl.addApplication(app1);
			pl.addApplication(app2);
			Assert.assertEquals(0, pl.length());
			pl.write(new byte[]{1});
			Assert.assertEquals(1, pl.length());
			pl.write(new byte[]{1});
			Assert.assertEquals(2, pl.length());
			
			pl.writeToDisk();
			Mockito.verify(kernel).copyTo(Matchers.eq(pl));
			Mockito.verify(app1).copyTo(Matchers.eq(pl));
			Mockito.verify(app2).copyTo(Matchers.eq(pl));
			Assert.assertEquals(130, pl.length());
			
			pl.close();
			Assert.assertEquals(130, image.length());
		} finally {
			image.delete();
		}
	}
}
