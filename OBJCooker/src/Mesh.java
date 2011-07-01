import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.StringReader;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;


public class Mesh {
	
	public final static int MAX_UVS = 8;

	class Vector {
		public float x, y, z;
	}
	
	class Color {
		public float r,g,b,a;
	}
	
	class UV {
		public float u,v;
	}
	
	class Index {
		public int p, u, n;
	}
	
	class Face {
		public Index i1, i2, i3;
		
		public Face( Index a, Index b, Index c ) {
			i1 = a;
			i2 = b;
			i3 = c;
		}
	}
	
	List< Vector > pos = new ArrayList< Vector >();
	List< UV> uv = new ArrayList< UV >();
	List< Vector > normal = new ArrayList< Vector >();
	
	List< Face > face = new ArrayList< Face >();
	
	
	public Mesh( String infile ) throws IOException {
		parse( infile );
	}
	
	public void parse( BufferedReader in ) throws IOException {
		
		
		while( in.ready() ) {
			StringTokenizer tk = new StringTokenizer( in.readLine() );
			
			while( tk.hasMoreTokens() ) {
				
				String s = tk.nextToken();
				
				if( s.equals( "v" ) ) {
					Vector v = new Vector();
					
					v.x = Float.parseFloat( tk.nextToken() );
					v.y = Float.parseFloat( tk.nextToken() );
					v.z = Float.parseFloat( tk.nextToken() );
					
					pos.add( v );
				}
				else if( s.equals( "vt" ) ) {
					UV v = new UV();
					
					v.u = Float.parseFloat( tk.nextToken() );
					v.v = Float.parseFloat( tk.nextToken() );
										
					uv.add( v );					
				}
				else if( s.equals( "vn" ) ) {
					Vector v = new Vector();
					
					v.x = Float.parseFloat( tk.nextToken() );
					v.y = Float.parseFloat( tk.nextToken() );
					v.z = Float.parseFloat( tk.nextToken() );
					
					normal.add( v );
				}
				else if( s.equals( "f" ) ) {
					
					Index idx[] = new Index[3];
					
					//read faces
					for( int i = 0; i < 3; ++i ) {
						idx[i] = new Index();
						
						idx[i].p = Integer.parseInt( tk.nextToken("/ ") );
						idx[i].u = Integer.parseInt( tk.nextToken("/ ") );
						idx[i].n = Integer.parseInt( tk.nextToken("/ ") );
					}					
					
					face.add( new Face( idx[0], idx[1], idx[2] ) );
				}
				else break;
			}
		}
	}
	
	public void parse( String infile ) throws IOException {
		
		parse( new BufferedReader( new FileReader( infile ) ) );
	}
	
	public void write( DataOutputStream out ) throws IOException {
		
		/*enum VertexField
		{
			VF_POSITION2D = 1,
			VF_POSITION3D = 2,
			VF_UV = 3,
			VF_UV_1 = 4,
			VF_UV_2 = 5,
			VF_UV_3 = 6,
			VF_UV_4 = 7,
			VF_UV_5 = 8,
			VF_UV_6 = 9,
			VF_UV_7 = 10,
			VF_COLOR = 11,
			VF_NORMAL = 12,
		};*/
		
		//write vertex number
		out.writeInt( pos.size() );
		
		//write index number
		out.writeInt( face.size()*3 );
		
		//set flags - by default only Pos3D, UV and NORMAL are enabled
		out.writeByte(0);  //2D
		out.writeByte(1);  //3D
		
		out.writeByte(1); //UVS
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		
		out.writeByte(0); //color
		
		out.writeByte(1); //normal
		
		//write down interleaved vertex data
		for( int i = 0; i < pos.size(); ++i ) {
			out.writeFloat( pos.get(i).x );
			out.writeFloat( pos.get(i).y );
			out.writeFloat( pos.get(i).z );
			
			out.writeFloat( uv.get(i).u );
			out.writeFloat( uv.get(i).v );
			
			out.writeFloat( pos.get(i).x );
			out.writeFloat( pos.get(i).y );
			out.writeFloat( pos.get(i).z );
		}
	}
	
	public void write( String outfile, ByteOrder endianness) throws IOException {
		
		//open file
		write( 
				new DataOutputStream( 
						new EndiannessFilterStream( 
								endianness, 
								new FileOutputStream( outfile ) ) ) );
	}
}
