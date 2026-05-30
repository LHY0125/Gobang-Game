import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { useRef } from 'react';
import type { CellState, Move } from '../../core/types';

interface Props { onBack: () => void; onStart: () => void; }

export default function LoadReplay({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const loadReplayBoard = useGameStore((s) => s.loadReplayBoard);
  const fileRef = useRef<HTMLInputElement>(null);

  const handleFile = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = () => {
      try {
        const json = JSON.parse(reader.result as string);
        const board: CellState[][] = Array.from({ length: json.board_size }, () =>
          Array(json.board_size).fill(0)
        );
        const moves: Move[] = [];
        for (const m of json.moves) {
          board[m.x][m.y] = (m.color === 'Black' ? 1 : 2) as CellState;
          moves.push({ position: { x: m.x, y: m.y }, color: m.color, turn: m.turn });
        }
        loadReplayBoard(board, moves);
        onStart();
      } catch {
        alert('无效的棋谱文件');
      }
    };
    reader.readAsText(file);
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.load_replay')}</h2>
      <input ref={fileRef} type="file" accept=".json" onChange={handleFile} />
      <button onClick={onBack} style={{ marginTop: 12 }}>返回</button>
    </div>
  );
}
