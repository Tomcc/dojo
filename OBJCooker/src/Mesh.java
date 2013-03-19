import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.StringTokenizer;

public class Mesh {
	
	class Vector {
		public float x, y, z;
		
		public Vector()
		{
			
		}
		
		public Vector( float x, float y, float z ) {
			this.x = x;
			this.y = y;
			this.z = z;
		}
	}
		
	class UV {
		public float u,v;
	}
	
	class Color
	{
		public int r,g,b,a;
		
		public Color( int r, int g, int b, int a ) {
			this.r = r;
			this.g = g;
			this.b = b;
			this.a = a;
		}
	}
	
	class Vertex {
		//the order in the bytestream must be:
		public Vector p;
		public Color color;
		public Vector n;
		public UV uv;	
		
		public int index = -1;
		
		public Vertex() {
			p = null;
			color = null;
			n = null;
			uv = null;
		}
		
		public boolean hasColor()	{	return color != null;	}
		public boolean hasNormal()	{	return n != null;	}
		public boolean hasUV()		{	return uv != null;	}
		
		public void writeTo( EndiannessFilterStream out ) throws IOException {
			
			//write out the existing attributes
			assert( p != null );
			out.writeFloat( p.x );
			out.writeFloat( p.y );
			out.writeFloat( p.z );
			
			if( color != null ) {
				out.write( color.r );
				out.write( color.g );
				out.write( color.b );
				out.write( color.a );
			}
			
			if( n != null ) {
				out.writeFloat( n.x );
				out.writeFloat( n.y );
				out.writeFloat( n.z );
			}
			
			if( uv != null ) {
				out.writeFloat( uv.u );
				out.writeFloat( uv.v );
			}
		}		
		
		public boolean equals( Object o ) {
			if( o.getClass() != this.getClass() )
				return false;
			
			Vertex v = (Vertex)o;
			
			//shallow comparisons are ok because attributes are unique
			return v.p == p && v.uv == uv && v.n == n && v.color == color;
		}
		
		public int hashCode() {
			return p.hashCode() + (uv != null ? uv.hashCode() : 0 ) + (color != null ? color.hashCode() : 0 ) + (n != null ? n.hashCode() : 0 );
		}
	}
	
	class Face {
		public int i1, i2, i3;
		
		public Face( int a, int b, int c ) {
			i1 = a;
			i2 = b;
			i3 = c;
		}
	}
	
	List< Vector > pos = new ArrayList< Vector >();
	List< UV> uv = new ArrayList< UV >();
	List< Vector > normal = new ArrayList< Vector >();
	List< Color > color = new ArrayList< Color >();
	
	List< Vertex > vertices = new ArrayList< Vertex >();
	HashSet< Vertex > verticesSet = new HashSet< Vertex >();
	
	List< Face > face = new ArrayList< Face >();
	
	Vector max = new Vector( -Float.MAX_VALUE, -Float.MAX_VALUE, -Float.MAX_VALUE );
	Vector min = new Vector( Float.MAX_VALUE, Float.MAX_VALUE, Float.MAX_VALUE );
	
	int positionIdx = -1;
	int colorIdx = -1;
	int normalIdx = -1;
	int uvIdx = -1;
	int assignedFieldIdx = 0;
	
	public Mesh( String infile ) throws IOException {
		parse( infile );
	}
	
	public int getVertexCount() {
		return vertices.size();
	}
	
	public int getIndexCount() {
		return face.size()*3;
	}
	
	public int addOrRetrieveIndex( Vertex v ) {
		
		 int idx = vertices.indexOf( v );
         
         if( idx == -1 ) { //new, add to the list
                 idx = vertices.size();
                 
                 vertices.add( v );
                 v.index = idx;
         }
         
         return idx;
	}
	
	public int parseVertex( StringTokenizer tk ) {
		Vertex v = new Vertex();
	
		String listString = tk.nextToken(" ") + "/"; //adds / to end the last index without special cases
		StringBuffer curIdx = new StringBuffer();
		int currentField = 0;
		for( int i = 0; i < listString.length(); ++i ) {
			
			char c = listString.charAt(i);
			//grab the current idx
			if( c == '/' )
			{
				if( curIdx.length() > 0 ) { //if it has no lenght, skip this field
				
					int idx = Integer.parseInt( curIdx.toString() )-1;
					
					if( currentField == positionIdx )
						v.p = pos.get( idx );
					else if( currentField == colorIdx )
						v.color = color.get( idx );
					else if( currentField == normalIdx )
						v.n = normal.get( idx );
					else if( currentField == uvIdx )
						v.uv = uv.get( idx );
					else
						assert( false ); //error!
				}
				
				curIdx = new StringBuffer();
				++currentField;
			}
			else
				curIdx.append( c );
		}
		
		//max and min
		max.x = Math.max( v.p.x, max.x );
		max.y = Math.max( v.p.y, max.y );
		max.z = Math.max( v.p.z, max.z );
		
		min.x = Math.min( v.p.x, min.x );
		min.y = Math.min( v.p.y, min.y );
		min.z = Math.min( v.p.z, min.z );
		
		//print some status
		if( vertices.size() % 100 == 0 )
			System.out.println( vertices.size() );
								
		//find its index
		return addOrRetrieveIndex( v );
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
					
					if( positionIdx < 0 )
						positionIdx = assignedFieldIdx++;  ///assign the order of this field
				}
				else if( s.equals( "vc" ) ) { //non-standard color format #1: vc r g b a
					int r = Integer.parseInt( tk.nextToken() );
					int g = Integer.parseInt( tk.nextToken() );
					int b = Integer.parseInt( tk.nextToken() );
					int a = Integer.parseInt( tk.nextToken() );
					
					color.add( new Color( r,g,b,a) );
					
					if( colorIdx < 0 )
						colorIdx = assignedFieldIdx++;
				}
				else if( s.equals( "vn" ) ) {
					Vector v = new Vector();
					
					v.x = Float.parseFloat( tk.nextToken() );
					v.y = Float.parseFloat( tk.nextToken() );
					v.z = Float.parseFloat( tk.nextToken() );
					
					normal.add( v );
					
					if( normalIdx < 0 )
						normalIdx = assignedFieldIdx++;
				}
				else if( s.equals( "vt" ) ) {
					UV v = new UV();
					
					v.u = Float.parseFloat( tk.nextToken() );
					v.v = 1.f - Float.parseFloat( tk.nextToken() );
										
					uv.add( v );
					
					if( uvIdx < 0 )
						uvIdx = assignedFieldIdx++;
				}
				else if( s.equals( "f" ) ) {
					
					int idx[] = new int[4];
					
					//read faces
					for( int i = 0; i < 3; ++i )
						idx[i] = parseVertex( tk );

					face.add( new Face( idx[0], idx[1], idx[2] ) );
					
					//is it a quad?
					if( tk.hasMoreTokens() ) {
						idx[3] = parseVertex( tk );
						
						face.add( new Face( idx[0], idx[2], idx[3] ) );		
					}
				}
				else break;
			}
		}
	}
	
	public void parse( String infile ) throws IOException {
		
		parse( new BufferedReader( new FileReader( infile ) ) );
	}
	
	public void write( EndiannessFilterStream out ) throws IOException {
		
		/*format is
		| byte index size | byte triangle mode | byte field 1 | ... | byte field N | Vector max | Vector min
		| vertex data | index data |
		
		where the fields are
		enum VertexField
		{
			VF_POSITION2D,
			VF_POSITION3D,
			VF_COLOR,
			VF_NORMAL,
			VF_UV,
			VF_UV_1,
		};*/
		
		//write down index size
		int indexBytes;
		
		if( getIndexCount() > 0xffff )
			indexBytes = 4;
		else if( getIndexCount() > 0xff )
			indexBytes = 2;
		else
			indexBytes = 1;
 
		out.write( indexBytes );
		
		//always triangle list
		out.write( 1 );
		
		//set flags - by default only Pos3D, UV and NORMAL are enabled
		out.write(0);  //2D
		out.write(1);  //3D
		
		//all the vertices MUST be the same, so the first vertex is ok to get the fields
		Vertex firstvertex = vertices.get(0);
		out.write( firstvertex.hasColor() ? 1 : 0 ); //COLOR
		out.write( firstvertex.hasNormal() ? 1 : 0 ); //NORMAL
		out.write( firstvertex.hasUV() ? 1 : 0 ); //UV0
		out.write(0); //UV1

		//max
		out.writeFloat( max.x );
		out.writeFloat( max.y );
		out.writeFloat( max.z );

		//min
		out.writeFloat( min.x );
		out.writeFloat( min.y );
		out.writeFloat( min.z );
				
		//write vertex number
		out.writeInt( getVertexCount() );
		
		//write index number
		out.writeInt( getIndexCount() );
				
		//write down interleaved vertex data
		for( Vertex v : vertices )
			v.writeTo( out );
		
		//write down index data
		if( indexBytes == 1 ) {
			for( Face f : face ) {
				out.write( f.i1 );
				out.write( f.i2 );
				out.write( f.i3 );
			}
		}
		else if( indexBytes == 2 ) {
			for( Face f : face ) {
				out.writeShort( (short)f.i1 );
				out.writeShort( (short)f.i2 );
				out.writeShort( (short)f.i3 );
			}
		}
		else if( indexBytes == 4 ) {
			for( Face f : face ) {
				out.writeInt( (int)f.i1 );
				out.writeInt( (int)f.i2 );
				out.writeInt( (int)f.i3 );
			}
		}
	}
	
	public void write( String outfile, ByteOrder endianness) throws IOException {
		
		//open file
		write(	new EndiannessFilterStream(	endianness,	new FileOutputStream( outfile ) ) );
	}
}
