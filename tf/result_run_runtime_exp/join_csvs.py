import pandas as pd
import sys

def format_run_cmd(cmd):
    d = cmd.split(' ')[1].split('/')
    if d[-2] == 'Benchs':
        return d[-1]
    return d[-2] + '/' + d[-1]

def main():
    dfOut = pd.read_csv('runP_1.csv', sep=',', index_col=0)

    dfP1 = pd.read_csv('runP_1.csv', sep=',', index_col=0)
    dfP2 = pd.read_csv('runP_2.csv', sep=',', index_col=0)
    dfP3 = pd.read_csv('runP_3.csv', sep=',', index_col=0)
    dfP4 = pd.read_csv('runP_4.csv', sep=',', index_col=0)
    dfP5 = pd.read_csv('runP_5.csv', sep=',', index_col=0)

    dfS1 = pd.read_csv('runS_1.csv', sep=',', index_col=0)
    dfS2 = pd.read_csv('runS_2.csv', sep=',', index_col=0)
    dfS3 = pd.read_csv('runS_3.csv', sep=',', index_col=0)
    dfS4 = pd.read_csv('runS_4.csv', sep=',', index_col=0)
    dfS5 = pd.read_csv('runS_5.csv', sep=',', index_col=0)

    dfOut = dfOut.drop(['JobRuntime'], axis=1)

    dfP1.rename(columns={'JobRuntime':'Parallel_Runtime_1'}, inplace=True)
    dfOut = dfOut.merge(dfP1, on='Command', how='right')
    dfP2.rename(columns={'JobRuntime':'Parallel_Runtime_2'}, inplace=True)
    dfOut = dfOut.merge(dfP2, on='Command', how='right')
    dfP3.rename(columns={'JobRuntime':'Parallel_Runtime_3'}, inplace=True)
    dfOut = dfOut.merge(dfP3, on='Command', how='right')
    dfP4.rename(columns={'JobRuntime':'Parallel_Runtime_4'}, inplace=True)
    dfOut = dfOut.merge(dfP4, on='Command', how='right')
    dfP5.rename(columns={'JobRuntime':'Parallel_Runtime_5'}, inplace=True)
    dfOut = dfOut.merge(dfP5, on='Command', how='right')


    dfS1.rename(columns={'JobRuntime':'Sequential_Runtime_1'}, inplace=True)
    dfOut = dfOut.merge(dfS1, on='Command', how='right')
    dfS2.rename(columns={'JobRuntime':'Sequential_Runtime_2'}, inplace=True)
    dfOut = dfOut.merge(dfS2, on='Command', how='right')
    dfS3.rename(columns={'JobRuntime':'Sequential_Runtime_3'}, inplace=True)
    dfOut = dfOut.merge(dfS3, on='Command', how='right')
    dfS4.rename(columns={'JobRuntime':'Sequential_Runtime_4'}, inplace=True)
    dfOut = dfOut.merge(dfS4, on='Command', how='right')
    dfS5.rename(columns={'JobRuntime':'Sequential_Runtime_5'}, inplace=True)
    dfOut = dfOut.merge(dfS5, on='Command', how='right')

    dfOut.to_csv(sys.stdout, index=False)

if __name__ == '__main__':
    if len(sys.argv) != 1:
        print ('Usage: python3 join_csvs.py')
    main()


