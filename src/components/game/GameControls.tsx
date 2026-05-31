import { useTranslation } from 'react-i18next';
import { invoke } from '@tauri-apps/api/core';
import { useGameStore } from '../../store/gameStore';

interface Props {
  onBackToMenu: () => void;
}

export default function GameControls({ onBackToMenu }: Props) {
  const { t } = useTranslation();
  const undo = useGameStore((s) => s.undo);
  const status = useGameStore((s) => s.status);
  const mode = useGameStore((s) => s.mode);
  const refreshBoard = useGameStore((s) => s.refreshBoard);

  const handleUndo = () => {
    undo(1);
  };

  const handleResign = async () => {
    await invoke('resign');
    await refreshBoard();
  };

  const handleSave = async () => {
    try {
      const json: string = await invoke('save_record');
      const blob = new Blob([json], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `gobang_${Date.now()}.json`;
      a.click();
      URL.revokeObjectURL(url);
    } catch (e) {
      console.error('保存棋谱失败:', e);
    }
  };

  return (
    <div className="game-controls">
      <button onClick={handleUndo} disabled={status === 'game_over' || mode === 'Online'}>
        {t('game.undo')}
      </button>
      <button onClick={handleResign} disabled={status === 'game_over'}>
        {t('game.resign')}
      </button>
      <button onClick={handleSave}>
        {t('game.save')}
      </button>
      <button onClick={onBackToMenu}>{t('game.new_game')}</button>
    </div>
  );
}
