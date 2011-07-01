import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteOrder;


public class EndiannessFilterStream extends OutputStream {
	
	private ByteOrder endianness, systemEndianness;
	OutputStream out;
	
	public EndiannessFilterStream( ByteOrder endianness, OutputStream out ) {
		
		this.endianness = endianness;
		this.out = out;
		
		systemEndianness = ByteOrder.nativeOrder();
	}
	
	@Override
	public void write(int arg0) throws IOException {
		//write, e basta
		out.write( arg0 );
	}
	
	boolean needsSwap()
	{
		return endianness != systemEndianness;
	}

	public void write(byte[] b, int off, int len) throws IOException {
		
		//swap endianness and write
		if( needsSwap() ) {
			
			for( int i = off+len-1; i >= off; --i )
				write( b[i] );
		}
		else {
			for( int i = off; i < off+len; ++i )
				write( b[i] );
		}
	}
	
	public void write(byte[] b )  throws IOException {
		write( b, 0, b.length );
	}
}
