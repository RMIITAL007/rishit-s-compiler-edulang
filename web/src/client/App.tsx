import React, { useState, useEffect } from 'react';
import { Header } from './components/Header';
import { Editor } from './components/Editor';
import { ResultsPanel } from './components/ResultsPanel';
import { SAMPLES, DEFAULT_SAMPLE } from './samples';
import { CompileResponse, CompileStatus, StageTab } from './types';

export const App: React.FC = () => {
  const [source, setSource] = useState<string>(DEFAULT_SAMPLE.code);
  const [selectedSampleId, setSelectedSampleId] = useState<string>(DEFAULT_SAMPLE.id);
  const [status, setStatus] = useState<CompileStatus>('idle');
  const [response, setResponse] = useState<CompileResponse | null>(null);
  const [activeTab, setActiveTab] = useState<StageTab>('output');
  const [errorMessage, setErrorMessage] = useState<string | null>(null);
  const [isBackendReady, setIsBackendReady] = useState<boolean>(false);

  // Check backend health on initial mount
  useEffect(() => {
    checkHealth();
  }, []);

  const checkHealth = async () => {
    try {
      const res = await fetch('/api/health');
      if (res.ok) {
        setIsBackendReady(true);
        if (status === 'offline') setStatus('idle');
      } else {
        setIsBackendReady(false);
        setStatus('offline');
      }
    } catch {
      setIsBackendReady(false);
      setStatus('offline');
    }
  };

  const handleSelectSample = (id: string) => {
    setSelectedSampleId(id);
    const sample = SAMPLES.find((s) => s.id === id);
    if (sample) {
      setSource(sample.code);
    }
  };

  const handleReset = () => {
    const sample = SAMPLES.find((s) => s.id === selectedSampleId);
    if (sample) {
      setSource(sample.code);
    } else {
      setSource(DEFAULT_SAMPLE.code);
    }
  };

  const handleCompile = async () => {
    if (!source.trim()) return;

    setStatus('compiling');
    setErrorMessage(null);

    try {
      const res = await fetch('/api/compile', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          source,
          mode: 'all',
        }),
      });

      if (!res.ok) {
        const errorData = await res.json().catch(() => null);
        setStatus('failure');
        setErrorMessage(errorData?.error || `Server returned HTTP ${res.status}: ${res.statusText}`);
        return;
      }

      const data: CompileResponse = await res.json();
      setResponse(data);

      if (data.timedOut) {
        setStatus('timeout');
      } else if (data.success && data.exitCode === 0) {
        setStatus('success');
      } else {
        setStatus('failure');
      }

      // Default to the output tab so user immediately sees results or diagnostics
      setActiveTab('output');
    } catch (err) {
      setStatus('offline');
      setErrorMessage(
        `Failed to communicate with compiler backend: ${(err as Error).message}. Check if port 5001 is listening.`
      );
    }
  };

  return (
    <div className="app-container">
      <Header status={status} isBackendReady={isBackendReady} />

      <main className="workbench">
        <Editor
          source={source}
          onChange={setSource}
          onCompile={handleCompile}
          onReset={handleReset}
          isCompiling={status === 'compiling'}
          selectedSampleId={selectedSampleId}
          onSelectSample={handleSelectSample}
        />

        <ResultsPanel
          response={response}
          status={status}
          activeTab={activeTab}
          onTabChange={setActiveTab}
          errorMessage={errorMessage}
        />
      </main>
    </div>
  );
};
