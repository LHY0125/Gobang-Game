import { useTranslation } from 'react-i18next';

interface Props {
  isPlaying: boolean;
  onTogglePlay: () => void;
  onPrev: () => void;
  onNext: () => void;
}

export default function ReplayControls({ isPlaying, onTogglePlay, onPrev, onNext }: Props) {
  const { t } = useTranslation();
  return (
    <div className="replay-controls">
      <button onClick={onPrev}>{t('replay.prev')}</button>
      <button onClick={onTogglePlay}>{isPlaying ? t('replay.pause') : t('replay.play')}</button>
      <button onClick={onNext}>{t('replay.next')}</button>
    </div>
  );
}
