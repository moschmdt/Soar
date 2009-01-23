package soar2d.players;

import soar2d.Soar2D;

/**
 * @author voigtjr
 *
 */
public class Taxi extends Player {	
	private MoveInfo move;

	public Taxi(String playerId) {
		super(playerId) ;
	}
	
	public void update(int [] location) {
		super.update(location);
	}

	public MoveInfo getMove() {
		return move;
	}

	public boolean getHumanMove() {
		move = Soar2D.wm.getHumanMove(this);

		if (move == null) {
			return false;
		}
		
		return true;
	}
}