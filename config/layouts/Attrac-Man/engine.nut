/////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - "Attrac-Man" Layout
//
// Copyright (c) 2013 Andrew Mickelson
//
// This program comes with ABSOLUTELY NO WARRANTY.  It is licensed under
// the terms of the GNU General Public License, version 3 or later.
//
// See Readme.md for credits and configuration instructions.
//
/////////////////////////////////////////////////////////

// Load playfield info
fe.do_nut("field.nut");

// Define the source file and dimensions for our sprites
const SpriteFile = "resource.png";
const SpriteSize = 32;

const BonusX = 28; const BonusY = 18;
const G0HomeX = 45; const G0HomeY = 3;
const G1HomeX = 9; const G1HomeY = 3;
const G2HomeX = 45; const G2HomeY = 37;
const G3HomeX = 9; const G3HomeY = 37;

const MinimumMove	= 0.1;

enum Direction
{
	Right	= 0,
	Left	= 1,
	Up		= 2,
	Down	= 3,
	None	= 4
}

enum GhostState
{
	Hidden				= 0,
	Chase					= 1,
	Scatter				= 2,
	Frightened			= 3,
	Dead					= 4
}

enum SGStatus
{
	Normal		= 0,
	Centre		= 1,
	Edge			= 2
}

function g2p( n ) // convert field (grid) coordinate to pixel coordinate
{
	return (4 + ( n * 8 ));
}

function p2g( n ) // convert pixel coordinate to field (grid) coordinate
{
	return ( n / 8 ).tointeger();
}

function reverse_direction( direction )
{
	switch ( direction )
	{
	case Direction.Up: return Direction.Down;
	case Direction.Down: return Direction.Up;
	case Direction.Left: return Direction.Right;
	case Direction.Right: return Direction.Left;
	default: return Direction.None;
	}
}

class SGResult
{
	status=SGStatus.Normal;
	remainder=0.0;

	function set( s, r )
	{
		status = s;
		remainder = r;
	}
}

class Sprite
{
	obj=null;
	my_x=0.0;
	my_y=0.0;

	constructor()
	{
		if ( ::texture == null )
			::texture <- obj = fe.add_image( SpriteFile, 0, 0, 16, 16 );
		else
			obj = fe.add_clone( ::texture );

		obj.subimg_width=obj.subimg_height=SpriteSize;
		obj.width=obj.height=16;
	}

	function set_pos( x, y )
	{
		my_x = x - obj.width / 2;
		my_y = y - obj.height / 2;
	}

	function pos_x()
	{
		return my_x + obj.width / 2;
	}

	function pos_y()
	{
		return my_y + obj.height / 2;
	}

	function _subgrid_move( advance, xf, yf )
	{
		local new_x = g2p( p2g( pos_x() ) ), new_y = g2p( p2g( pos_y() ) );
		local ret_val = SGResult();

		local sub = xf * ( pos_x() - new_x ) + yf * ( pos_y() - new_y );
		local new_sub = sub + advance;

		// check if we've hit the centre
		if (( new_sub == 0 ) || ( sub * new_sub < 0 ))
		{
			xf = yf = 0;
			ret_val.set( SGStatus.Centre, fabs( new_sub ) );
		}
		else
		{
			// Don't move beyond edge of next block
			if ( new_sub < -4.0 )
			{
				ret_val.set( SGStatus.Edge, fabs( new_sub + 4.0 ) );
				new_sub = -4.5;
			}
			else if ( new_sub > 3.5 )
			{
				ret_val.set( SGStatus.Edge, new_sub - 3.5 );
				new_sub = 4.0;
			}
		}

		if ( xf )
		{
			new_x += new_sub * xf;

			// handle tunnel wrap around
			//
			if ( new_x < 0 )
				new_x = fe.layout.width - advance * xf;
			else if ( new_x > fe.layout.width )
				new_x = advance * xf;
		}

		if ( yf )
		{
			new_y += new_sub * yf;

			if ( new_y < 0 )
				new_y = fe.layout.height - advance * yf;
			else if ( new_y > fe.layout.height )
				new_y = advance * yf;
		}

		set_pos( new_x, new_y );
		return ret_val;
	}

	function subgrid_move( advance )
	{
		switch ( direction )
		{
		case Direction.Left:
			return _subgrid_move( -advance, 1, 0 );

		case Direction.Right:
			return _subgrid_move(  advance, 1, 0 );

		case Direction.Up:
			return _subgrid_move( -advance, 0, 1 );

		case Direction.Down:
			return _subgrid_move(  advance, 0, 1 );
		}
		return SGResult();
	}

	function collision( other )
	{
		return ( ( p2g( pos_x() ) == p2g( other.pos_x() ) )
				&& ( p2g( pos_y() ) == p2g( other.pos_y() ) ) );
	}

	function _score( curr_gx, curr_gy, target_gx, target_gy )
	{
		return ( pow( curr_gy - target_gy, 2 ) 
				+ pow( curr_gx - target_gx, 2 ) );
	}

	//
	// figure out the best way to turn to get to specified grid target
	// (if target coordinates are -1,-1, turn a random available direction)
	//
	function get_best_direction( target_gx, target_gy, curr_direction )
	{
		//
		// Pick travel direction to reach given grid target
		//
		local curr_gx = p2g( pos_x() ), curr_gy = p2g( pos_y() );
		local best_score = 100000, best_direction = curr_direction;
		local temp = 0;

		if (( ::field[ curr_gy ][ curr_gx - 1 ] )
				&& ( curr_direction != Direction.Right ))
		{
			if ( target_gx == -1 )
				best_score = rand() % 1000;
			else
				best_score = _score( curr_gx - 1, curr_gy, target_gx, target_gy );
	
			best_direction = Direction.Left;
		}

		if (( ::field[ curr_gy ][ curr_gx + 1 ] )
				&& ( curr_direction != Direction.Left ))
		{
			if ( target_gx == -1 )
				temp = rand() % 1000;
			else
				temp = _score( curr_gx + 1, curr_gy, target_gx, target_gy );

			if ( temp < best_score )
			{
				best_score = temp;
				best_direction = Direction.Right;
			}
		}
		if (( ::field[ curr_gy - 1 ][ curr_gx ] )
			&& ( curr_direction != Direction.Down ))
		{
			if ( target_gx == -1 )
				temp = rand() % 1000;
			else
				temp = _score( curr_gx, curr_gy - 1, target_gx, target_gy );

			if ( temp < best_score )
			{
				best_score = temp;
				best_direction = Direction.Up;
			}
		}
		if (( ::field[ curr_gy + 1 ][ curr_gx ] )
				&& ( curr_direction != Direction.Up ))
		{
			if ( target_gx == -1 )
				temp = rand() % 1000;
			else
				temp = _score( curr_gx, curr_gy + 1, target_gx, target_gy );

			if ( temp < best_score )
			{
				best_score = temp;
				best_direction = Direction.Down;
			}
		}

		return best_direction;
	}
}

class Bonus extends Sprite
{
	up=true;
	hide_frame=0;

	constructor()
	{
		base.constructor()
		set_pos( g2p( BonusX ), g2p( BonusY ) );
		obj.x = my_x;
		obj.y = my_y;
	}

	function eaten( frame )
	{
		up = false;
		hide_frame = frame + 20;
		obj.subimg_x = ( 8 + ( ::level % 4 ) ) * SpriteSize;
		obj.subimg_y = 3 * SpriteSize;
	}

	function animate( frame )
	{
		if (( hide_frame ) && ( hide_frame < frame ))
		{
			// hide
			obj.visible = false;	
			hide_frame = 0;
		}
	}

	function set_up( u )
	{
		up = u;
		obj.visible = up;

		if ( up )
		{
			obj.subimg_x = ( ::level % 8 ) * SpriteSize;
			obj.subimg_y = 5 * SpriteSize;
		}
	}
}

class Dot extends Sprite
{
	constructor()
	{
		base.constructor()
		obj.subimg_x= 13 * SpriteSize;
		obj.subimg_y= 5 * SpriteSize;
	}

	function set_pos( x, y )
	{
		base.set_pos( x, y );
		obj.x = my_x;
		obj.y = my_y;
	}
}

class Energizer extends Sprite
{
	up=true;

	constructor()
	{
		base.constructor()
		obj.subimg_x= 12 * SpriteSize;
		obj.subimg_y= 5 * SpriteSize;
	}

	function set_pos( x, y )
	{
		base.set_pos( x, y );
		obj.x = my_x;
		obj.y = my_y;
	}

	function animate( frame )
	{
		if (( up ) && (( frame / 3 ) % 2 ))
			obj.visible = true;
		else
			obj.visible = false;
	}

	function set_up( u )
	{
		up = u;
	}
}

class Player extends Sprite
{
	die=0; // The frame number that the player died on, 0 if alive
	direction=Direction.None;
	speed=0.8;
	cruise_control=false; // false if user provided input during this life
	cruise_x=-1;
	cruise_y=-1;
	is_mirror=false;

	constructor()
	{
		base.constructor()
		obj.subimg_y=0;
		obj.subimg_x = 2 * SpriteSize;
	}

	function init()
	{
		die=0;
		speed = 0.8;
		set_pos( g2p( ::PlayerStart[0] ), g2p( ::PlayerStart[1] ) );

		if ( cruise_control )
			direction = get_best_direction( -1, -1, Direction.None );
		else
			direction = Direction.Left;

		cruise_control = true;
		cruise_x=cruise_y=-1;
	}

	function death( frame )
	{
		if ( die == 0 )
		{
			die = frame;
			set_sound( "chase", false );
			set_sound( "fright", false );
			set_sound( "death", true );
		}
	}

	function animate( frame )
	{
		obj.visible = true;
		if ( die > 0 )
		{
			local df = ( frame - die ) / 3;
			if ( df > 11 )
				::reset_actors = true; // flag to reset player/ghosts
			else
				obj.subimg_x = _mirror(( 2 + df ) * SpriteSize);
		}
		else if ( direction != Direction.None )
			obj.subimg_x = _mirror(( frame % 3 ) * SpriteSize);
	}

	function _mirror( x)
	{
		if ( is_mirror )
		{
			if ( obj.subimg_width > 0 )
				obj.subimg_width = -(SpriteSize);
			x = x + SpriteSize;
		}
		else if ( obj.subimg_width < 0 )
		{
			obj.subimg_width = SpriteSize;
		}
		return x;
	}

	function move( f_adv )
	{
		// Don't move if we're dying
		if ( die > 0 )
			return;

		_move( f_adv * speed * ::speed );

		switch ( direction )
		{
		case Direction.Right:
			is_mirror = false;
			obj.rotation = 0; obj.x = my_x; obj.y = my_y; break;

		case Direction.Left:
			is_mirror = true;
			obj.rotation = 0; obj.x = my_x; obj.y = my_y; break;

		case Direction.Up:
			is_mirror = false;
			obj.rotation = 270; obj.x = my_x; obj.y = my_y + 15; break;

		case Direction.Down:
			is_mirror = true;
			obj.rotation = 270; obj.x = my_x; obj.y = my_y + 15; break;
		}
	}

	function _move( f_adv )
	{
		local retval = subgrid_move( f_adv );
		local x = p2g( pos_x() ), y = p2g( pos_y() );

		if ( retval.status == SGStatus.Edge )
		{
			// Moved to new tile, consume dots/energizers
			//
			if ( ::dots.rawin(y) && ::dots[y].rawin(x) )
			{
				if ( ::dots[y][x].obj.visible == true )
				{
					::dots[y][x].obj.visible = false;
					::dots_up--;
					::dots_changed = true;
				}
			}
			if ( ::energizers.rawin(y) && ::energizers[y].rawin(x) )
			{
				if ( ::energizers[y][x].up == true )
				{
					::energizers[y][x].set_up( false );
					::dots_up--;
					::dots_changed = true;
					::ate_energizer = true;
				}
			}

			if (( cruise_control ) && ( do_cruise_control() ))
				retval.remainder = 0;
		}
		else if ( retval.status == SGStatus.Centre )
		{
			try
			{
				// confirm that we can still move forward
				switch ( direction )
				{
				case Direction.Up:
					if ( ::field[ y - 1 ][ x ]  == 0 )
						direction = Direction.None;
					break;

				case Direction.Down:
					if ( ::field[ y + 1 ][ x ]  == 0 )
						direction = Direction.None;
					break;

				case Direction.Left:
					if (( x > 0 ) && ( ::field[ y ][ x - 1 ]  == 0 ))
						direction = Direction.None;
					break;

				case Direction.Right:
					if ( ( x < ::GridSize[0] ) && ( ::field[ y ][ x + 1 ]  == 0 ))
						direction = Direction.None;
					break;
				}
			}
			catch (e)
			{
				direction = Direction.None;
			}
		}

		if (( direction != Direction.None ) 
				&& ( retval.remainder > MinimumMove ))
			return _move( retval.remainder );
	}

	function update_direction( ttime )
	{
		if ( fe.get_input_state( AM_CONFIG["p1_up"] ) )
		{
			if ( ::field[ p2g( pos_y() ) - 1 ][ p2g( pos_x() ) ] )
				direction = Direction.Up;

			cruise_control = false;
		}
		else if ( fe.get_input_state( AM_CONFIG["p1_down"] ) )
		{
			if ( ::field[ p2g( pos_y() ) + 1 ][ p2g( pos_x() ) ] )
				direction = Direction.Down;

			cruise_control = false;
		}
		else if ( fe.get_input_state( AM_CONFIG["p1_left"] ) )
		{
			local temp = p2g( pos_x() ) - 1;
			if (( temp < 0 ) // special case for tunnel movement
					|| ( ::field[ p2g( pos_y() ) ][ temp ] ))
				direction = Direction.Left;

			cruise_control = false;
		}
		else if ( fe.get_input_state( AM_CONFIG["p1_right"] ) )
		{
			local temp = p2g( pos_x() ) + 1;
			if (( temp > ::GridSize[0] ) // special case for tunnel movement
					|| ( ::field[ p2g( pos_y() ) ][ temp ] ))
				direction = Direction.Right;

			cruise_control = false;
		}
	}

	//
	// AI functions to use when there is no user input:
	//

	// Check for monster in area in front of player given the grid position
	// x,y and direction dir.  Return dir if no monster, return the reverse
	// direction if there is a monster.
	//
	function check_fright( x, y, dir )
	{
		local lx=x - 2, ly=y - 2, ux=x + 2, uy=y + 2, op=dir;

		switch ( dir )
		{
		case Direction.Up: uy=y, ly=y-6, op=Direction.Down; break;
		case Direction.Down: uy=y+6, ly=y, op=Direction.Up; break;
		case Direction.Left: ux=x, lx=x-6, op=Direction.Right; break;
		case Direction.Right: ux=x+6, lx=x, op=Direction.Left; break;
		}

		foreach ( g in ::ghosts )
		{
			if ((( g.gstate == GhostState.Chase ) 
					|| ( g.gstate == GhostState.Scatter ))
				&& ( p2g( g.pos_x() ) > lx )
				&& ( p2g( g.pos_x() ) < ux )
				&& ( p2g( g.pos_y() ) > ly )
				&& ( p2g( g.pos_y() ) < uy ))
			{
				return op;
			}
		}
		return dir;
	}

	function get_cruise_target( x, y )
	{
		// find new target
		// divide into quadrants, visit ghosts decision points with dots
		local lx=0, ly=0, ux=::field[0].len(), uy=::field.len();

		if ( x > ux/2 )
			lx= ux/2 + 1;
		else
			ux=ux/2;

		if ( y > uy/2 )
			ly=uy/2 + 1;
		else
			uy=uy/2;

		local do_any=false;
		if ( rand() % 5 == 0 ) // 20% of the time go to any random decision pt
			do_any=true;
		
		local nodes = [], others = [];
		for ( local i=ly; i<uy; i++ )
		{
			for ( local j=lx; j<ux; j++ )
			{
				if (( ::dots.rawin(i) )
					&& ( ::dots[i].rawin(j) )
					&& ( ::dots[i].rawin(j) )
					&& ( ( do_any ) || ( ::dots[i][j].obj.visible == true ) ))
				{
					if ( ::field[i][j] & 4 ) 
						nodes.append( [ j, i ] );
					else if ( !do_any )
						others.append( [ j, i ] );
				}
			}
		}
			
		if ( nodes.len() > 0 )
			return nodes[ rand() % nodes.len() ];
		else if ( others.len() > 0 )
			return others[ rand() % others.len() ];
		else
			return null;
	}

	// returns true to clear any remaining movement this turn
	function do_cruise_control( x=-1, y=-1 )
	{
		// AI for player where no user input
		local gx = x, gy = y, retval = false;
		if ( x < 0 )
			gx = p2g( pos_x() );
		if ( y < 0 )
			gy = p2g( pos_y() );

		// Reverse course if a ghost is in our face
		local fd = check_fright( gx, gy, direction )
		if ( fd != direction )
		{
			if ( check_fright( gx, gy, fd ) == fd )
			{
				direction = fd;
				retval = true;
				cruise_x = -1; // get a new target when frightened
			}
		}

		if ((( gx == cruise_x ) && ( gy == cruise_y ))
			|| ( cruise_x == -1 ))
		{
			local target = get_cruise_target( gx, gy );

			if ( target != null )
			{
				cruise_x = target[0];
				cruise_y = target[1];
			}
			else if ( ::dots_up > 0 )
			{
				while ( target == null )
				{
					// random to another quadrant
					local i = rand() % 4;
					switch ( i )
					{
					case 0:
						cruise_x=G0HomeX; cruise_y=G0HomeY; break;
					case 1:
						cruise_x=G1HomeX; cruise_y=G1HomeY; break;
					case 2:
						cruise_x=G2HomeX; cruise_y=G2HomeY; break;
					case 3:
						cruise_x=G3HomeX; cruise_y=G3HomeY; break;
					}
					target = get_cruise_target( cruise_x, cruise_y );
				}
				cruise_x = target[0];
				cruise_y = target[1];
			}
		}

		// otherwise pick a direction if we are at a ghost decision point
		if (( gx >= 0 ) && ( gx < ::GridSize[0] )
				&& ( gy >= 0 ) && ( gy < ::GridSize[1] ) 
				&& ( ::field[gy][gx] & 4 )) 
		{
			if ( ::field[gy][gx] == 7 )
				direction = get_best_direction( -1, -1, direction );
			else
				direction = get_best_direction( cruise_x, cruise_y, direction );
		}

		return retval;
	}
}

class Ghost extends Sprite
{
	gnum=0;
	gstate=GhostState.Hidden;
	direction=Direction.None;
	next_direction=Direction.None;
	speed=0.75;
	eyes_frame=0;
	reverse=false;
	collided=false;

	constructor( n )
	{
		gnum=n;
		base.constructor();
		obj.subimg_y = ( gnum + 1 ) * SpriteSize;
	}

	function init()
	{
		gstate=::global_state;
		direction=Direction.Up;
		next_direction=Direction.None;
		reverse=false;
		collided=false;
		speed=0.75;
	}

	function eaten( frame, number )
	{
		gstate = GhostState.Dead;
		eyes_frame = frame + 20;

		// show the score
		obj.subimg_x = ( 8 + number ) * SpriteSize;
		obj.subimg_y = 4 * SpriteSize;
	}

	function set_pos( x, y )
	{
		base.set_pos( x, y );
		obj.x = my_x;
		obj.y = my_y;
	}

	function animate( frame )
	{
		if ( gstate != GhostState.Hidden )
			obj.visible = true;

		switch ( gstate )
		{
		case GhostState.Chase:
		case GhostState.Scatter:
			obj.subimg_x = ( direction * 2 + ( ( frame / 3 ) % 2 ) ) * SpriteSize;
			obj.subimg_y = ( gnum + 1 ) * SpriteSize;
			break;

		case GhostState.Frightened:

			if (( ::fright_change - frame ) < 50 )
			{
				obj.subimg_x = ( 8 + ( ( frame / 3 ) % 4 ) ) * SpriteSize;
				obj.subimg_y = SpriteSize;
			}
			else
			{
				obj.subimg_x = ( 8 + ( ( frame / 3 ) % 2 ) ) * SpriteSize;
				obj.subimg_y = SpriteSize;
			}
			break;

		case GhostState.Dead:
			if ( eyes_frame == -1 ) // eyes
			{
				obj.subimg_x = ( 8 + direction ) * SpriteSize;
				obj.subimg_y = 2 * SpriteSize;
			}
			else if ( frame >= eyes_frame ) // switch to showing eyes
			{
				eyes_frame = -1;
				speed = 2.0;
			}
			break;

		case GhostState.Hidden:
			obj.visible = false;
			break;
		}
	}

	function move( f_adv )
	{
		// Don't move if we commit murder
		if ( ::pman.die > 0 )
		{
			if ( obj.visible )
			{
				gstate = GhostState.Hidden;
				obj.visible = false;
			}
			return;
		}

		if ( gstate == GhostState.Dead )
		{
			if ( eyes_frame != -1 ) // Don't move if we're showing the score
				return;

			// Stop moving if we've hit the ghost house
			//
			if (( p2g( pos_x() ) == ::GhostHouse[0] )
					&& ( p2g( pos_y() ) == ::GhostHouse[1] ))
			{
				gstate = GhostState.Hidden;
				set_pos( g2p( ::GhostHouse[0] ), g2p( ::GhostHouse[1] + 1 ) );
			}
		}

		if ( gstate == GhostState.Hidden )
			return;

		_move( f_adv * speed * ::speed );
	}

	function _move( f_adv )
	{
		local retval = subgrid_move( f_adv );

		local x = p2g( pos_x() ), y = p2g( pos_y() );

		if ( retval.status == SGStatus.Centre )
		{
			if ( reverse )
			{
				// If the reverse flag is set, then reverse course
				direction = reverse_direction( direction );
				reverse = false;
			}
			else if (( x < 0 ) || ( x > ::GridSize[0] ) || ( !( ::field[y][x] & 4 ) ))
			{
				// nothing
			}
			else
			{
				// We've hit a ghost decision point
				//
				local up=fe.get_input_state( AM_CONFIG["p2_up"] );
				local down=fe.get_input_state( AM_CONFIG["p2_down"] );
				local left=fe.get_input_state( AM_CONFIG["p2_left"] );
				local right=fe.get_input_state( AM_CONFIG["p2_right"] );
				local control_move=false;

				if ( ( gnum == 0 ) && ( up || down || left || right ))
				{
					// respond to the ghost controls if pressed
					if ( left && ::field[y][x-1] )
					{
						direction = Direction.Left;
						control_move = true;
					}
					else if ( right && ::field[y][x+1] )
					{
						direction = Direction.Right;
						control_move = true;
					}
					else if ( up && ::field[y-1][x] )
					{
						direction = Direction.Up;
						control_move = true;
					}
					else if ( down && ::field[y+1][x] )
					{
						direction = Direction.Down;
						control_move = true;
					}
				}

				if ( !control_move )
				{
					// Pick next direction for ghost movement
					// Start by getting grid target...
					//
					local target_gx = -1, target_gy = -1;
					switch ( gstate )
					{
					case GhostState.Chase:
						// g0 targets pman directly
						target_gx = p2g( ::pman.pos_x() );
						target_gy = p2g( ::pman.pos_y() );

						if ( gnum == 3 )
						{
							// g3 chases pman until 8 tiles away, then goes home
							if (( abs( target_gx - p2g( pos_x() ) )
								+ abs( target_gy - p2g( pos_y() ) ) ) < 8 )
							{
								target_gx = G3HomeX;
								target_gy = G3HomeY;
							}
						}
						else
						{
							// g1 targets the spot 3 ahead of pman, g2 uses the spot 2
							// ahead
							local look_ahead=0;
							switch (gnum)
							{
							case 1:
								look_ahead = 3; break;
							case 2:
								look_ahead = 2; break;
							default: // look_ahead = 0
							}

							switch ( ::pman.direction )
							{
							case Direction.Up:
								target_gy -= look_ahead; break;
							case Direction.Down:
								target_gy += look_ahead; break;
							case Direction.Left:
								target_gx -= look_ahead; break;
							case Direction.Right:
								target_gx += look_ahead; break;
							}

							if ( gnum == 2 )
							{
								// g2 tries to circle around pman opposite g0
								local diff_x = target_gx - p2g( ::ghosts[0].pos_x() );
								local diff_y = target_gy - p2g( ::ghosts[0].pos_y() );
								target_gx += diff_x;
								target_gy += diff_y;
							}
						}
						break;

					case GhostState.Scatter:
						switch ( gnum )
						{
						case 0:
							target_gx = G0HomeX; target_gy = G0HomeY; break;

						case 1:
							target_gx = G1HomeX; target_gy = G1HomeY; break;

						case 2:
							target_gx = G2HomeX; target_gy = G2HomeY; break;

						case 3:
							target_gx = G3HomeX; target_gy = G3HomeY; break;
						}
						break;

					case GhostState.Dead:
						target_gx = ::GhostHouse[0]; target_gy = ::GhostHouse[1]; break;
					}

					direction = get_best_direction( target_gx, target_gy, direction );
				}
			}
		}

		if ( collision( ::pman ))
			collided = true;

		if  ( retval.remainder > MinimumMove )
			return _move( retval.remainder );
	}
}

function state_update( ttime, animate_frame )
{
	// Check if we need to reset the game actors
	//
	if ( ::reset_actors )
	{
		::pman.init();
		foreach ( g in ::ghosts )
		{
			g.gstate = GhostState.Hidden;
			g.set_pos( g2p( ::GhostHouse[0] ), g2p( ::GhostHouse[1] + 1 ) );
		}

		::last_ghost = ttime - 3000;
		::reset_actors = false;
	}
	
	// Release a ghost from the ghost house every 3 seconds
	//
	if ( ttime - ::last_ghost > 3000 )
	{
		foreach ( g in ::ghosts )
		{
			if ( g.gstate == GhostState.Hidden )
			{
				g.init();
				::last_ghost = ttime;
				break;
			}
		}
	}

	// Cycle ghosts between scatter (7 secs) and chase modes (20 secs)
	//
	local gm_time = ( ttime / 1000 ) % 27;
	if ( gm_time < 8 )
	{
		if ( ::global_state != GhostState.Scatter )
		{
			::global_state = GhostState.Scatter;

			foreach ( g in ::ghosts )
			{
				if ( g.gstate == GhostState.Chase )
				{
					g.gstate = GhostState.Scatter;
					g.reverse = true;
				}
			}
		}
	}
	else
	{
		if ( ::global_state != GhostState.Chase )
		{
			::global_state = GhostState.Chase;

			foreach ( g in ::ghosts )
			{
				if ( g.gstate == GhostState.Scatter )
				{
					g.gstate = GhostState.Chase;
					g.reverse = true;
				}
			}
		}
	}

	//
	// Check if ghosts need to switch back from frightened
	//
	if (( ::fright_change != 0 )
		&& ( animate_frame > ::fright_change ))
	{
		foreach ( g in ::ghosts )
		{
			if ( g.gstate == GhostState.Frightened )
			{
				g.gstate = ::global_state;
				g.speed = 0.75;
			}
		}

		::fright_change = 0;
		::pman.speed = 0.8;
	}

	//
	// Check things that depend on dots/energizers being eaten
	//
	if ( ::dots_changed )
	{
		if ( ::ate_energizer )
		{
			foreach ( g in ::ghosts )
			{
				if ( g.gstate == ::global_state )
				{
					g.gstate = GhostState.Frightened;
					g.speed = 0.5;
					g.reverse = true;
				}
			}

			::pman.speed = 0.9;
			::fright_change = animate_frame + 100;
			::eat_count = 0;
			::ate_energizer = false;
		}

		switch ( ::dots_up )
		{
			case 0:
				::level++;
				maze_init();
				break;

			case 80:
				::bonus.set_up( true );
				break;
		}
		::dots_changed = false;
	}
}

function set_sound( name, value )
{
	if ( ::sounds.rawin( name ) )
	{
		if ( ::sounds[name].playing != value )
			::sounds[name].playing = value;

		// set the position of the sound to match the current pos
		// of the player
		if ( value && ( name != "intro" ))
		{
			::sounds[name].x = (::pman.my_x.tofloat()/fe.layout.width) - 0.5;
			::sounds[name].y = (::pman.my_y.tofloat()/fe.layout.height) - 0.5;
		}
	}
}

function maze_init()
{
	::dots_up = 0;

	for ( local r=0; r < ::field.len(); r++ )
	{
		for ( local c=0; c < ::field[r].len(); c++ )
		{
			switch ( ::field[r][c] )
			{
			case 2:		// dot
			case 6:
			case 7:
				if ( !::dots.rawin( r ) )
					::dots[r] <- {};

				if ( !::dots[r].rawin( c ) )
				{
					::dots[r][c] <- Dot();
					::dots[r][c].set_pos( g2p( c ), g2p( r ) );
				}
				::dots[r][c].obj.visible = true;
				::dots_up++;
				break;

			case 3:		// energizer
				if ( !::energizers.rawin( r ) )
					::energizers[r] <- {};

				if ( !::energizers[r].rawin( c ) )
				{
					::energizers[r][c] <- Energizer();
					::energizers[r][c].set_pos( g2p( c ), g2p( r ) );
				}

				::energizers[r][c].set_up( true );
				::dots_up++;
				break;
			}
		}
	}

	if ( !::rawin( "bonus" ) )
		::bonus <- Bonus();

	::bonus.set_up( false );

	if ( !::rawin( "pman" ) )
		::pman <- Player();

	::reset_actors = true;

	if ( !::rawin( "ghosts" ) )
		::ghosts <- [ Ghost( 0 ), Ghost( 1 ), Ghost( 2 ), Ghost( 3 ) ];

	::pman.set_pos( g2p( ::PlayerStart[0] ), g2p( ::PlayerStart[1] ) );
	::pman.obj.x = ::pman.my_x; ::pman.obj.y = ::pman.my_y;

	set_sound( "chase", false );
	set_sound( "fright", false );
	set_sound( "intro", true );
}

//
// Adjust speed if user is pressing the speed up/down keys
//
function speed_adjust()
{
	local speed_up = fe.get_input_state( AM_CONFIG["speed_up"] );
	local speed_down = fe.get_input_state( AM_CONFIG["speed_down"] );

	if ( ::query_speed == false )
	{
		if ( ( speed_up == false ) && ( speed_down == false ) )
			::query_speed = true; 
	}
	else
	{
		if ( speed_up && ( ::speed < 2.0 ) )
		{
			::speed += SPEED_INCREMENT;
			::query_speed = false;
		}
		else if ( speed_down && ( ::speed > 0.5 ) )
		{
			::speed -= SPEED_INCREMENT;
			::query_speed = false;
		}
	}
}

//
// Global variables
//
::texture <- null;	// image that gets cloned for each sprite
::speed <- DEFAULT_SPEED;
::query_speed <- true;
::eat_count <- 0;		// how many ghosts have been eaten with this energizer
::global_state <- GhostState.Scatter;
::dots <- {};
::energizers <- {};
::dots_up <- 0;
::dots_changed <- true;
::ate_energizer <- false;
::fright_change <- 0;
::level <- 0;
::last_ghost <- -3001;	// the time the last ghost was released
::reset_actors <- true;
::last_frame <- 0;
::sounds <- {};

if ( AM_CONFIG["intro_sound"].len() > 0 )
	::sounds["intro"] <- fe.add_sound( AM_CONFIG["intro_sound"] );

if ( AM_CONFIG["death_sound"].len() > 0 )
	::sounds["death"] <- fe.add_sound( AM_CONFIG["death_sound"] );

if ( AM_CONFIG["chase_sound"].len() > 0 )
{
	::sounds["chase"] <- fe.add_sound( AM_CONFIG["chase_sound"] );
	::sounds["chase"].loop = true;
}

if ( AM_CONFIG["fright_sound"].len() > 0 )
{
	::sounds["fright"] <- fe.add_sound( AM_CONFIG["fright_sound"] );
	::sounds["fright"].loop = true;
}

maze_init();

fe.add_ticks_callback( "tick" );

//
// this function will be called repeatedly by the frontend 
// to run the game
//
function tick( ttime )
{
	local frame = ttime / 8.3; // 8.3=1000/120

	if ( ::sounds.rawin("intro") && ::sounds["intro"].playing )
	{
		::last_frame = frame;
		return;
	}
	else if ( !::sounds.rawin("death") || !::sounds["death"].playing )
	{
		local in_fright = ( ::pman.speed > 0.85 );
		set_sound( "fright", in_fright );
		set_sound( "chase", !in_fright );
	}

	::pman.update_direction( ttime );
	speed_adjust();

	if ( frame > ::last_frame + 1 )
	{
		// cap catch up for gaps greater than 0.5 seconds
		if (( frame - ::last_frame ) > 60 )
			::last_frame = frame - 60;

		local animate_frame = ( frame / 6 ).tointeger();

		state_update( ttime, animate_frame );

		//
		// Do movements
		//
		local advance_stock = ( frame - ::last_frame ) * ::speed;

		while ( advance_stock > 2.0 )
		{
			::pman.move( 2.0 );
			foreach ( g in ::ghosts )
				g.move( 2.0 );

			advance_stock -= 2.0;
		}

		::last_frame = frame;

		::pman.move( advance_stock );
		foreach ( g in ::ghosts )
			g.move( advance_stock );

		//
		// Do animation and state updates
		//
		::pman.animate( animate_frame );

		foreach ( g in ::ghosts )
		{
			g.animate( animate_frame );

			if ( g.collided == true )
			{
				if (( ::pman.die == 0 ) 
					&& ( g.gstate == GhostState.Frightened ))
				{
					g.eaten( animate_frame, ::eat_count );
					::eat_count++;
					::pman.obj.visible = false;
					::last_frame += 50; // 1 second global pause
				}
				else if (( g.gstate == GhostState.Chase )
						|| ( g.gstate == GhostState.Scatter ))
					::pman.death( animate_frame );

				g.collided = false;
			}
		}

		::bonus.animate( animate_frame );
		if ( ::bonus.up && ::bonus.collision( ::pman ) )
			::bonus.eaten( animate_frame );

		foreach ( e in ::energizers )
		{
			foreach ( ee in e )
				ee.animate( animate_frame );
		}
	}
}
