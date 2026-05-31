import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import LocalGameSetup from './LocalGameSetup';
import AiGameSetup from './AiGameSetup';
import OnlineSetup from './OnlineSetup';
import LoadReplay from './LoadReplay';

type View = 'main' | 'local' | 'ai' | 'online' | 'replay';

interface Props {
  onGameStart: () => void;
  onReplayStart: () => void;
}

export default function MainMenu({ onGameStart, onReplayStart }: Props) {
  const { t } = useTranslation();
  const [view, setView] = useState<View>('main');

  if (view === 'local') return <LocalGameSetup onBack={() => setView('main')} onStart={onGameStart} />;
  if (view === 'ai') return <AiGameSetup onBack={() => setView('main')} onStart={onGameStart} />;
  if (view === 'online') return <OnlineSetup onBack={() => setView('main')} onStart={onGameStart} />;
  if (view === 'replay') return <LoadReplay onBack={() => setView('main')} onStart={onReplayStart} />;

  return (
    <div className="main-menu">
      <h1 className="menu-title">{t('app.title')}</h1>
      <div className="menu-buttons">
        <button onClick={() => setView('local')}>{t('menu.local_game')}</button>
        <button onClick={() => setView('ai')}>{t('menu.ai_game')}</button>
        <button
          onClick={() => setView('online')}
          disabled
          title={t('menu.online_game_disabled')}
        >
          {t('menu.online_game')} (开发中)
        </button>
        <button onClick={() => setView('replay')}>{t('menu.load_replay')}</button>
      </div>
    </div>
  );
}
