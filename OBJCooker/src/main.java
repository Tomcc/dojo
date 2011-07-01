import java.io.IOException;
import java.nio.ByteOrder;


public class main {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		/*if( args.length < 2 ) {
			System.out.println( "usage: 'OBJCooker infile outfile");
			return;
		}*/
				
		String infile = "dado.obj";
		String outfile = "dado.dbm";
		
		Mesh m;
		long t;
		try {

			System.out.println( "parsing file " + infile );
			
			t = System.currentTimeMillis();
			m = new Mesh( infile );
			
			System.out.println( "done... " + (System.currentTimeMillis()-t) + " ms" );
			System.out.println( "writing binary file " + outfile );

			t = System.currentTimeMillis();
			
			m.write( "dado.lem", ByteOrder.LITTLE_ENDIAN );
			m.write( "dado.bem", ByteOrder.BIG_ENDIAN );
			
			System.out.println( "DONE... " + (System.currentTimeMillis()-t) + " ms" );
			
		} catch (IOException e) {
			System.out.println( "ERROR: " + e.getMessage() );
		}
	}
}
