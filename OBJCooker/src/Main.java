import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.nio.ByteOrder;

import javax.swing.JFileChooser;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Node;


public class Main {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		String infile = "";
		if( args.length < 1 )	{
			JFileChooser chooser = new JFileChooser(".");
			chooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
		
			if( chooser.showOpenDialog( null ) != JFileChooser.APPROVE_OPTION )
				return;
			
			infile = chooser.getSelectedFile().getAbsolutePath();
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
		
			t = System.currentTimeMillis();
			
			m.write( filename + ".mesh", ByteOrder.LITTLE_ENDIAN );
			
			System.out.println( (System.currentTimeMillis()-t) + " ms" );
			
			System.out.println( "DONE" );
			
		} catch (IOException e) {
			System.out.println( "ERROR: " + e.getMessage() );
		}
	}
}
