import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteOrder;


public class Main {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		String infile = "";
		if( args.length < 1 )	{
			System.out.print( "Input file name: ");
			BufferedReader in = new BufferedReader( new InputStreamReader( System.in ) );
			
			try {
				infile = in.readLine();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		else
			infile = args[0];
						
		String filename = infile.substring(0, infile.lastIndexOf('.') );
		
		Mesh m;
		long t;
		try {

			System.out.println( "parsing file " + infile + "... \t" );
			
			t = System.currentTimeMillis();
			m = new Mesh( infile );
			
			t = System.currentTimeMillis()-t;
			
			System.out.print( "Parsed " + m.getVertexCount() + " vertices and " + m.getIndexCount() + " indices... ");
			System.out.println( t + " ms" );
		
			System.out.print( "writing big-endian mesh " + filename + ".bem... \t" );
			t = System.currentTimeMillis();
			
			m.write( filename + ".bem", ByteOrder.BIG_ENDIAN );
			
			System.out.println( (System.currentTimeMillis()-t) + " ms" );			
			System.out.print( "writing little-endian mesh " + filename + ".lem... \t" );
			t = System.currentTimeMillis();
			
			m.write( filename + ".lem", ByteOrder.LITTLE_ENDIAN );
			
			System.out.println( (System.currentTimeMillis()-t) + " ms" );
			
			System.out.println( "DONE" );
			
		} catch (IOException e) {
			System.out.println( "ERROR: " + e.getMessage() );
		}
	}
}
