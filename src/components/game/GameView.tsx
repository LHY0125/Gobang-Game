import BoardCanvas from '../board/BoardCanvas';
import GameInfo from './GameInfo';
import GameControls from './GameControls';
import TimerDisplay from './TimerDisplay';

interface Props {
  onBackToMenu: () => void;
}

export default function GameView({ onBackToMenu }: Props) {
  return (
    <div className="game-view">
      <GameInfo />
      <div className="board-container">
        <BoardCanvas />
      </div>
      <TimerDisplay />
      <GameControls onBackToMenu={onBackToMenu} />
    </div>
  );
}
