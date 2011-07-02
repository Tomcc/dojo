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
		return !endianness.equals( systemEndianness );
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
	
	public void writeInt( int i ) throws IOException {
	
		byte b[] = new byte[4];
		
		//build an array
		b[0] =(byte)( i >> 24 );
		b[1] =(byte)( (i << 8) >> 24 );
		b[2] =(byte)( (i << 16) >> 24 );
		b[3] =(byte)( (i << 24) >> 24 );
		
		write( b );
	}
	
	public void writeFloat( float f ) throws IOException {
		
		writeInt( Float.floatToIntBits( f ) );
	}
}
