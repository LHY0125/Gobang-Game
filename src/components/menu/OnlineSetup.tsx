import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { useState } from 'react';
import type { GameConfig } from '../../core/types';

interface Props { onBack: () => void; onStart: () => void; }

export default function OnlineSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const [ip, setIp] = useState('');

  const baseConfig: GameConfig = {
    boardSize: 15, useForbiddenRules: true, useTimer: false,
    timeLimitSecs: 60, aiDifficulty: 3, playerColor: 'Black', isServer: false,
    remoteAddress: '',
  };

  const handleHost = async () => {
    await startGame('Online', { ...baseConfig, isServer: true });
    onStart();
  };

  const handleJoin = async () => {
    await startGame('Online', { ...baseConfig, remoteAddress: ip });
    onStart();
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.online_game')}</h2>
      <button onClick={handleHost}>{t('menu.host_room')}</button>
      <div style={{ display: 'flex', gap: 8, marginTop: 12 }}>
        <input value={ip} onChange={(e) => setIp(e.target.value)} placeholder={t('menu.ip_placeholder') as string} />
        <button onClick={handleJoin} disabled={!ip}>{t('menu.join_room')}</button>
      </div>
      <button onClick={onBack} style={{ marginTop: 12 }}>{t('common.back')}</button>
    </div>
  );
}
