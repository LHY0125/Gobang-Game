import { useState } from 'react';
import MainMenu from './components/menu/MainMenu';
import GameView from './components/game/GameView';
import ReplayView from './components/replay/ReplayView';
import ErrorBoundary from './components/common/ErrorBoundary';
import './App.css';

type Page = 'menu' | 'game' | 'replay';

function App() {
  const [page, setPage] = useState<Page>('menu');

  const handleGameStart = () => setPage('game');
  const handleReplayStart = () => setPage('replay');
  const handleBackToMenu = () => setPage('menu');

  let content: React.ReactNode;
  if (page === 'game') {
    content = <GameView onBackToMenu={handleBackToMenu} />;
  } else if (page === 'replay') {
    content = <ReplayView onBackToMenu={handleBackToMenu} />;
  } else {
    content = (
      <MainMenu
        onGameStart={handleGameStart}
        onReplayStart={handleReplayStart}
      />
    );
  }

  return <ErrorBoundary>{content}</ErrorBoundary>;
}

export default App;
