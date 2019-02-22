library(ggplot2)
library(dplyr)
library(tibble)
library(readr)
library(tidyr)
runtime_data_tmp <- read.csv("Desktop/TaskMiner/scripts/R_scripts/data.csv")
runtime_data <- runtime_data_tmp[order(runtime_data_tmp$Speedup),]
BENCH <- runtime_data[1:21,1]
BENCH <- cbind(BENCH,(runtime_data[1:21,7:11] / runtime_data[1:21,2:6]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + coord_flip()
p + ggsave('Downloads/boxplot_group1.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE)

BENCH <- runtime_data[22:42,1]
BENCH <- cbind(BENCH,(runtime_data[22:42,7:11] / runtime_data[22:42,2:6]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + coord_flip()
p + ggsave('Downloads/boxplot_group2.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE)

BENCH <- runtime_data[43:63,1]
BENCH <- cbind(BENCH,(runtime_data[43:63,7:11] / runtime_data[43:63,2:6]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + coord_flip()
p + ggsave('Downloads/boxplot_group3.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE)

BENCH <- runtime_data[64:83,1]
BENCH <- cbind(BENCH,(runtime_data[64:83,7:11] / runtime_data[64:83,2:6]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + coord_flip()
p + ggsave('Downloads/boxplot_group4.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE)

BENCH <- runtime_data[,1]
BENCH <- cbind(BENCH,(runtime_data[,7:11] / runtime_data[,2:6]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + coord_flip()  +
  theme(text=element_text(size=8,  family="Comic Sans MS"))
p + ggsave('Downloads/boxplot.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE) 

BENCH <- runtime_data[,1]
BENCH <- cbind(BENCH,(runtime_data[,7:11] / runtime_data[,2:6]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + 
  theme(text=element_text(size=8,  family="Comic Sans MS"))
p + ggsave('Downloads/boxplot2.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE) 
