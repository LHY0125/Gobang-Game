import { useState } from 'react';
import MainMenu from './components/menu/MainMenu';
import GameView from './components/game/GameView';
import ReplayView from './components/replay/ReplayView';
import './App.css';

type Page = 'menu' | 'game' | 'replay';

function App() {
  const [page, setPage] = useState<Page>('menu');

  const handleGameStart = () => setPage('game');
  const handleReplayStart = () => setPage('replay');
  const handleBackToMenu = () => setPage('menu');

  if (page === 'game') return <GameView onBackToMenu={handleBackToMenu} />;
  if (page === 'replay') return <ReplayView onBackToMenu={handleBackToMenu} />;

  return (
    <MainMenu
      onGameStart={handleGameStart}
      onReplayStart={handleReplayStart}
    />
  );
}

export default App;
